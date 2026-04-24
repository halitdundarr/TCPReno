#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <arpa/inet.h>

#include "simulator.h"
#include "tcp_algorithms.h"

static int parse_event(const char *token, EventType *event) {
    if (strcmp(token, "ACK_NEW") == 0) {
        *event = EVENT_ACK_NEW;
        return 0;
    }
    if (strcmp(token, "ACK_DUP") == 0) {
        *event = EVENT_ACK_DUP;
        return 0;
    }
    if (strcmp(token, "TIMEOUT") == 0) {
        *event = EVENT_TIMEOUT;
        return 0;
    }
    return -1;
}

int load_events(const char *scenario_path, PacketEvent *events, int max_events) {
    FILE *file = fopen(scenario_path, "r");
    char line[MAX_LINE];
    int count = 0;
    int line_no = 0;
    int last_step = -1;
    if (!file) return -1;

    while (fgets(line, sizeof(line), file)) {
        int step = 0;
        char event_name[32];
        EventType parsed_event;
        char *cursor = line;
        line_no++;
        while (isspace((unsigned char)*cursor)) {
            cursor++;
        }
        if (*cursor == '#' || *cursor == '\0' || *cursor == '\n') {
            continue;
        }
        if (count >= max_events) {
            fprintf(stderr, "Scenario parse error: too many events (max %d)\n", max_events);
            fclose(file);
            return -1;
        }
        if (sscanf(cursor, "%d %31s", &step, event_name) != 2) {
            fprintf(stderr, "Scenario parse error at line %d: expected '<step> <event>'\n", line_no);
            fclose(file);
            return -1;
        }
        if (step <= 0) {
            fprintf(stderr, "Scenario parse error at line %d: step must be positive\n", line_no);
            fclose(file);
            return -1;
        }
        if (step <= last_step) {
            fprintf(
                stderr,
                "Scenario parse error at line %d: step %d is not strictly increasing (previous %d)\n",
                line_no,
                step,
                last_step
            );
            fclose(file);
            return -1;
        }
        if (parse_event(event_name, &parsed_event) != 0) {
            fprintf(
                stderr,
                "Scenario parse error at line %d: unknown event '%s'\n",
                line_no,
                event_name
            );
            fclose(file);
            return -1;
        }
        events[count].step = step;
        events[count].type = parsed_event;
        last_step = step;
        count++;
    }

    fclose(file);
    return count;
}

static void apply_algorithm(TcpAlgorithm algo, SenderState *sender, EventType event) {
    if (algo == ALGO_RENO) {
        apply_reno(sender, event);
    } else {
        apply_tahoe(sender, event);
    }
}

SimulationMetrics run_simulation(
    TcpAlgorithm algo,
    PacketEvent *events,
    int event_count,
    CwndLog *logs,
    int *log_count
) {
    SenderState sender;
    SimulationMetrics metrics;
    double cwnd_sum = 0.0;
    double max_cwnd = 0.0;
    int i;
    int recovery_start = -1;
    int recovery_end = -1;

    init_sender(&sender);
    *log_count = event_count;

    for (i = 0; i < event_count; i++) {
        if (events[i].type == EVENT_TIMEOUT || events[i].type == EVENT_ACK_DUP) {
            if (recovery_start < 0 && sender.loss_events == 0) {
                recovery_start = events[i].step;
            }
        }
        apply_algorithm(algo, &sender, events[i].type);

        logs[i].step = events[i].step;
        logs[i].event = events[i].type;
        logs[i].cwnd = sender.cwnd;
        logs[i].ssthresh = sender.ssthresh;
        logs[i].state = sender.state;

        cwnd_sum += sender.cwnd;
        if (sender.cwnd > max_cwnd) max_cwnd = sender.cwnd;

        if (recovery_start >= 0 && recovery_end < 0 && sender.cwnd >= sender.ssthresh) {
            recovery_end = events[i].step;
        }
    }

    metrics.avg_cwnd = event_count > 0 ? cwnd_sum / event_count : 0.0;
    metrics.max_cwnd = max_cwnd;
    metrics.throughput = event_count > 0 ? (double)sender.acked_packets / event_count : 0.0;
    metrics.recovery_steps = (recovery_start >= 0 && recovery_end > recovery_start)
                                 ? (recovery_end - recovery_start)
                                 : -1;
    return metrics;
}

void print_simulation_log(const CwndLog *logs, int log_count) {
    int i;
    printf("step\tevent\t\tcwnd\tssthresh\tstate\n");
    for (i = 0; i < log_count; i++) {
        printf(
            "%d\t%-10s\t%.2f\t%.2f\t\t%s\n",
            logs[i].step,
            event_to_str(logs[i].event),
            logs[i].cwnd,
            logs[i].ssthresh,
            state_to_str(logs[i].state)
        );
    }
}

void print_metrics(const SimulationMetrics *metrics) {
    printf("\n=== Metrics ===\n");
    printf("Average cwnd: %.2f\n", metrics->avg_cwnd);
    printf("Max cwnd: %.2f\n", metrics->max_cwnd);
    printf("Throughput (acked/step): %.2f\n", metrics->throughput);
    if (metrics->recovery_steps >= 0) {
        printf("Recovery steps: %d\n", metrics->recovery_steps);
    } else {
        printf("Recovery steps: N/A\n");
    }
}

static const NodeConfig *find_node_const(const NodeConfig *nodes, int node_count, char node) {
    int i;
    for (i = 0; i < node_count; i++) {
        if (nodes[i].node == node) return &nodes[i];
    }
    return NULL;
}

static const RouteEntry *find_route(const NodeConfig *cfg, char destination) {
    int i;
    for (i = 0; i < cfg->route_count; i++) {
        if (cfg->routes[i].destination == destination) return &cfg->routes[i];
    }
    return NULL;
}

static int port_for_node(const NodeConfig *nodes, int node_count, char node_name) {
    int i;
    for (i = 0; i < node_count; i++) {
        if (nodes[i].node == node_name) return nodes[i].port;
    }
    return -1;
}

static int parse_packet(const char *packet, char *src, char *dst, char *msg, int msg_size) {
    char local_src = 0;
    char local_dst = 0;
    char local_msg[512];
    if (sscanf(packet, "%c|%c|%511[^\n]", &local_src, &local_dst, local_msg) != 3) {
        return -1;
    }
    *src = local_src;
    *dst = local_dst;
    strncpy(msg, local_msg, (size_t)(msg_size - 1));
    msg[msg_size - 1] = '\0';
    return 0;
}

static int send_packet_to_port(int sockfd, int port, const char *packet) {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    return (int)sendto(sockfd, packet, strlen(packet), 0, (struct sockaddr *)&addr, sizeof(addr));
}

int load_topology(const char *topology_dir, NodeConfig *nodes, int *node_count) {
    const char all_nodes[] = {'A', 'B', 'C', 'D', 'E', 'F'};
    int ports[] = {5001, 5002, 5003, 5004, 5005, 5006};
    int i;

    *node_count = 0;
    for (i = 0; i < NODE_COUNT; i++) {
        char path[256];
        FILE *file;
        NodeConfig *cfg;
        snprintf(path, sizeof(path), "%s/%c.conf", topology_dir, all_nodes[i]);
        file = fopen(path, "r");
        if (!file) return -1;

        cfg = &nodes[*node_count];
        cfg->node = all_nodes[i];
        cfg->port = ports[i];
        cfg->route_count = 0;

        while (!feof(file) && cfg->route_count < MAX_ROUTES) {
            char line[MAX_LINE];
            RouteEntry entry;
            char next_hop_str[8];
            if (!fgets(line, sizeof(line), file)) break;
            if (line[0] == '#' || strlen(line) < 5) continue;
            if (sscanf(
                    line,
                    " %c %7s %d %63[^\n]",
                    &entry.destination,
                    next_hop_str,
                    &entry.cost,
                    entry.path
                ) == 4) {
                entry.next_hop = (strcmp(next_hop_str, "-") == 0) ? '-' : next_hop_str[0];
                cfg->routes[cfg->route_count++] = entry;
            }
        }

        fclose(file);
        (*node_count)++;
    }
    return 0;
}

int run_forwarding_demo(
    const NodeConfig *nodes,
    int node_count,
    char source,
    char destination,
    const char *message
) {
    char current = (char)toupper((unsigned char)source);
    char target = (char)toupper((unsigned char)destination);
    int hops = 0;

    while (current != target && hops < 16) {
        const NodeConfig *cfg = find_node_const(nodes, node_count, current);
        const RouteEntry *route;
        if (!cfg) return -1;
        route = find_route(cfg, target);
        if (!route || route->next_hop == '-') return -1;

        if (hops == 0) {
            printf("[%c] Destination %c, next hop %c\n", current, target, route->next_hop);
        } else {
            printf(
                "[%c] Forwarding message from %c to %c, next hop %c\n",
                current,
                source,
                destination,
                route->next_hop
            );
        }

        current = route->next_hop;
        hops++;
    }

    if (current == target) {
        printf("[%c] Received message from %c: %s\n", target, source, message);
        return 0;
    }
    return -1;
}

int run_node_process(const char *topology_dir, char node_name) {
    NodeConfig nodes[NODE_COUNT];
    int node_count = 0;
    const NodeConfig *self_cfg;
    int my_port;
    int sockfd;
    struct sockaddr_in bind_addr;

    if (load_topology(topology_dir, nodes, &node_count) != 0) {
        fprintf(stderr, "Failed to load topology configs from %s\n", topology_dir);
        return -1;
    }

    node_name = (char)toupper((unsigned char)node_name);
    self_cfg = find_node_const(nodes, node_count, node_name);
    if (!self_cfg) {
        fprintf(stderr, "Node %c not found in topology.\n", node_name);
        return -1;
    }
    my_port = self_cfg->port;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons((unsigned short)my_port);
    bind_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(sockfd, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) != 0) {
        perror("bind");
        close(sockfd);
        return -1;
    }

    printf("[%c] Node started on port %d. Use: send <DEST> <MESSAGE>\n", node_name, my_port);
    fflush(stdout);

    for (;;) {
        fd_set readfds;
        int maxfd = sockfd > STDIN_FILENO ? sockfd : STDIN_FILENO;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sockfd, &readfds);

        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char line[512];
            char command[16];
            char dest = 0;
            char message[400];
            const RouteEntry *route;
            int next_port;
            char packet[512];

            if (!fgets(line, sizeof(line), stdin)) break;
            if (sscanf(line, "%15s", command) != 1) continue;
            if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) break;
            if (sscanf(line, "send %c %399[^\n]", &dest, message) != 2) {
                fprintf(stderr, "[%c] Invalid command. Use: send <DEST> <MESSAGE>\n", node_name);
                continue;
            }

            dest = (char)toupper((unsigned char)dest);
            route = find_route(self_cfg, dest);
            if (!route || route->next_hop == '-') {
                fprintf(stderr, "[%c] No route to %c\n", node_name, dest);
                continue;
            }
            next_port = port_for_node(nodes, node_count, route->next_hop);
            if (next_port < 0) {
                fprintf(stderr, "[%c] Unknown next hop %c\n", node_name, route->next_hop);
                continue;
            }

            printf("[%c] Destination %c, next hop %c\n", node_name, dest, route->next_hop);
            snprintf(packet, sizeof(packet), "%c|%c|%s", node_name, dest, message);
            if (send_packet_to_port(sockfd, next_port, packet) < 0) {
                perror("sendto");
            }
            fflush(stdout);
        }

        if (FD_ISSET(sockfd, &readfds)) {
            char packet[512];
            ssize_t n = recv(sockfd, packet, sizeof(packet) - 1, 0);
            if (n > 0) {
                char src;
                char dst;
                char msg[400];
                const RouteEntry *route;
                int next_port;

                packet[n] = '\0';
                if (parse_packet(packet, &src, &dst, msg, sizeof(msg)) != 0) continue;

                if (node_name == dst) {
                    printf("[%c] Received message from %c: %s\n", node_name, src, msg);
                    fflush(stdout);
                    continue;
                }

                route = find_route(self_cfg, dst);
                if (!route || route->next_hop == '-') {
                    fprintf(stderr, "[%c] Dropping packet for %c (no route)\n", node_name, dst);
                    continue;
                }
                next_port = port_for_node(nodes, node_count, route->next_hop);
                if (next_port < 0) {
                    fprintf(stderr, "[%c] Dropping packet for %c (bad next hop)\n", node_name, dst);
                    continue;
                }

                printf("[%c] Forwarding message from %c to %c, next hop %c\n", node_name, src, dst, route->next_hop);
                if (send_packet_to_port(sockfd, next_port, packet) < 0) perror("sendto");
                fflush(stdout);
            }
        }
    }

    close(sockfd);
    return 0;
}
