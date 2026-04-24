#include "tcp_algorithms.h"

static double half_cwnd(double cwnd) {
    double value = cwnd / 2.0;
    return value < 2.0 ? 2.0 : value;
}

void apply_reno(SenderState *sender, EventType event) {
    switch (event) {
        case EVENT_ACK_NEW:
            if (sender->in_fast_recovery) {
                sender->cwnd = sender->ssthresh;
                sender->in_fast_recovery = 0;
                sender->state = STATE_CONGESTION_AVOIDANCE;
            } else if (sender->cwnd < sender->ssthresh) {
                sender->state = STATE_SLOW_START;
                sender->cwnd += 1.0;
            } else {
                sender->state = STATE_CONGESTION_AVOIDANCE;
                sender->cwnd += (1.0 / sender->cwnd);
            }
            sender->dup_ack_count = 0;
            sender->acked_packets++;
            break;

        case EVENT_ACK_DUP:
            sender->dup_ack_count++;
            if (sender->in_fast_recovery) {
                sender->cwnd += 1.0;
            } else if (sender->dup_ack_count == 3) {
                sender->ssthresh = half_cwnd(sender->cwnd);
                sender->cwnd = sender->ssthresh + 3.0;
                sender->in_fast_recovery = 1;
                sender->state = STATE_FAST_RECOVERY;
                sender->loss_events++;
            }
            break;

        case EVENT_TIMEOUT:
            sender->ssthresh = half_cwnd(sender->cwnd);
            sender->cwnd = 1.0;
            sender->dup_ack_count = 0;
            sender->in_fast_recovery = 0;
            sender->state = STATE_SLOW_START;
            sender->loss_events++;
            break;

        default:
            break;
    }
}
