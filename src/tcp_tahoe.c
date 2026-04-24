#include "tcp_algorithms.h"

static void on_new_ack(SenderState *sender) {
    sender->dup_ack_count = 0;
    if (sender->cwnd < sender->ssthresh) {
        sender->state = STATE_SLOW_START;
        sender->cwnd += 1.0;
    } else {
        sender->state = STATE_CONGESTION_AVOIDANCE;
        sender->cwnd += (1.0 / sender->cwnd);
    }
    sender->acked_packets++;
}

static void on_loss(SenderState *sender) {
    if (sender->cwnd < 2.0) {
        sender->ssthresh = 2.0;
    } else {
        sender->ssthresh = sender->cwnd / 2.0;
        if (sender->ssthresh < 2.0) {
            sender->ssthresh = 2.0;
        }
    }
    sender->cwnd = 1.0;
    sender->dup_ack_count = 0;
    sender->in_fast_recovery = 0;
    sender->state = STATE_SLOW_START;
    sender->loss_events++;
}

void apply_tahoe(SenderState *sender, EventType event) {
    switch (event) {
        case EVENT_ACK_NEW:
            on_new_ack(sender);
            break;
        case EVENT_ACK_DUP:
            sender->dup_ack_count++;
            if (sender->dup_ack_count >= 3) {
                on_loss(sender);
            }
            break;
        case EVENT_TIMEOUT:
            on_loss(sender);
            break;
        default:
            break;
    }
}
