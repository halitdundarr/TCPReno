#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "simulator.h"

static void usage(const char *prog) {
    printf("Usage:\n");
    printf("  %s --algo reno|tahoe --scenario <path>\n", prog);
    printf("  %s --topology-demo <src> <dst> <message>\n", prog);
    printf("  %s --node <A|B|C|D|E|F> [--topology-dir <path>]\n", prog);
}

static int parse_algo(const char *name, TcpAlgorithm *algo) {
    if (strcmp(name, "reno") == 0) {
        *algo = ALGO_RENO;
        return 0;
    }
    if (strcmp(name, "tahoe") == 0) {
        *algo = ALGO_TAHOE;
        return 0;
    }
    return -1;
}

int main(int argc, char **argv) {
    const char *topology_dir = "topology";
    const char *algo_name = NULL;
    const char *scenario_path = NULL;
    const char *demo_message = NULL;
    char node_name = '\0';
    char demo_source = '\0';
    char demo_destination = '\0';
    enum { MODE_NONE, MODE_SIM, MODE_DEMO, MODE_NODE } mode = MODE_NONE;
    int i;
    if (argc < 2) {
        usage(argv[0]);
        return 1;
    }

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--topology-dir") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing value for --topology-dir\n");
                usage(argv[0]);
                return 1;
            }
            topology_dir = argv[++i];
            continue;
        }
        if (strcmp(argv[i], "--algo") == 0) {
            if (mode != MODE_NONE && mode != MODE_SIM) {
                fprintf(stderr, "Conflicting mode arguments.\n");
                usage(argv[0]);
                return 1;
            }
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing value for --algo\n");
                usage(argv[0]);
                return 1;
            }
            mode = MODE_SIM;
            algo_name = argv[++i];
            continue;
        }
        if (strcmp(argv[i], "--scenario") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing value for --scenario\n");
                usage(argv[0]);
                return 1;
            }
            scenario_path = argv[++i];
            continue;
        }
        if (strcmp(argv[i], "--node") == 0) {
            if (mode != MODE_NONE && mode != MODE_NODE) {
                fprintf(stderr, "Conflicting mode arguments.\n");
                usage(argv[0]);
                return 1;
            }
            if (i + 1 >= argc || argv[i + 1][0] == '\0') {
                fprintf(stderr, "Missing value for --node\n");
                usage(argv[0]);
                return 1;
            }
            mode = MODE_NODE;
            node_name = argv[++i][0];
            continue;
        }
        if (strcmp(argv[i], "--topology-demo") == 0) {
            if (mode != MODE_NONE && mode != MODE_DEMO) {
                fprintf(stderr, "Conflicting mode arguments.\n");
                usage(argv[0]);
                return 1;
            }
            if (i + 3 >= argc) {
                fprintf(stderr, "Missing values for --topology-demo\n");
                usage(argv[0]);
                return 1;
            }
            mode = MODE_DEMO;
            demo_source = argv[++i][0];
            demo_destination = argv[++i][0];
            demo_message = argv[++i];
            continue;
        }
        fprintf(stderr, "Unknown argument: %s\n", argv[i]);
        usage(argv[0]);
        return 1;
    }

    if (mode == MODE_NODE) {
        if (run_node_process(topology_dir, node_name) != 0) {
            fprintf(stderr, "Node process failed.\n");
            return 1;
        }
        return 0;
    }

    if (mode == MODE_DEMO) {
        NodeConfig nodes[NODE_COUNT];
        int node_count = 0;
        if (load_topology(topology_dir, nodes, &node_count) != 0) {
            fprintf(stderr, "Failed to load topology configs.\n");
            return 1;
        }
        if (run_forwarding_demo(nodes, node_count, demo_source, demo_destination, demo_message) != 0) {
            fprintf(stderr, "Forwarding demo failed.\n");
            return 1;
        }
        return 0;
    }

    if (mode != MODE_SIM || !algo_name || !scenario_path) {
        usage(argv[0]);
        return 1;
    }

    {
        TcpAlgorithm algo;
        PacketEvent events[MAX_EVENTS];
        CwndLog logs[MAX_EVENTS];
        int event_count = load_events(scenario_path, events, MAX_EVENTS);
        int log_count = 0;
        SimulationMetrics metrics;
        if (parse_algo(algo_name, &algo) != 0) {
            fprintf(stderr, "Invalid algorithm: %s (allowed: reno|tahoe)\n", algo_name);
            usage(argv[0]);
            return 1;
        }
        if (event_count < 0) {
            fprintf(stderr, "Could not parse scenario file: %s\n", scenario_path);
            return 1;
        }

        metrics = run_simulation(algo, events, event_count, logs, &log_count);
        print_simulation_log(logs, log_count);
        print_metrics(&metrics);
    }

    return 0;
}
