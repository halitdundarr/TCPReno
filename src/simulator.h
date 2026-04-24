#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "tcp_state.h"

int load_events(const char *scenario_path, PacketEvent *events, int max_events);
SimulationMetrics run_simulation(
    TcpAlgorithm algo,
    PacketEvent *events,
    int event_count,
    CwndLog *logs,
    int *log_count
);
void print_simulation_log(const CwndLog *logs, int log_count);
void print_metrics(const SimulationMetrics *metrics);

int load_topology(const char *topology_dir, NodeConfig *nodes, int *node_count);
int run_forwarding_demo(
    const NodeConfig *nodes,
    int node_count,
    char source,
    char destination,
    const char *message
);
int run_node_process(const char *topology_dir, char node_name);

#endif
