#ifndef TCP_STATE_H
#define TCP_STATE_H

#define MAX_EVENTS 2048
#define MAX_LINE 256
#define MAX_ROUTES 32
#define NODE_COUNT 6

typedef enum {
    ALGO_TAHOE = 0,
    ALGO_RENO = 1
} TcpAlgorithm;

typedef enum {
    STATE_SLOW_START = 0,
    STATE_CONGESTION_AVOIDANCE = 1,
    STATE_FAST_RECOVERY = 2
} CongestionState;

typedef enum {
    EVENT_ACK_NEW = 0,
    EVENT_ACK_DUP = 1,
    EVENT_TIMEOUT = 2
} EventType;

typedef struct {
    int step;
    EventType type;
} PacketEvent;

typedef struct {
    double cwnd;
    double ssthresh;
    int dup_ack_count;
    int in_fast_recovery;
    CongestionState state;
    int acked_packets;
    int loss_events;
} SenderState;

typedef struct {
    int step;
    EventType event;
    double cwnd;
    double ssthresh;
    CongestionState state;
} CwndLog;

typedef struct {
    char destination;
    char next_hop;
    int cost;
    char path[64];
} RouteEntry;

typedef struct {
    char node;
    int port;
    int route_count;
    RouteEntry routes[MAX_ROUTES];
} NodeConfig;

typedef struct {
    double avg_cwnd;
    double max_cwnd;
    double throughput;
    int recovery_steps;
} SimulationMetrics;

const char *event_to_str(EventType event);
const char *state_to_str(CongestionState state);
void init_sender(SenderState *sender);

#endif
