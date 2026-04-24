#include "tcp_state.h"

const char *event_to_str(EventType event) {
    switch (event) {
        case EVENT_ACK_NEW:
            return "ACK_NEW";
        case EVENT_ACK_DUP:
            return "ACK_DUP";
        case EVENT_TIMEOUT:
            return "TIMEOUT";
        default:
            return "UNKNOWN_EVENT";
    }
}

const char *state_to_str(CongestionState state) {
    switch (state) {
        case STATE_SLOW_START:
            return "SLOW_START";
        case STATE_CONGESTION_AVOIDANCE:
            return "CONGESTION_AVOIDANCE";
        case STATE_FAST_RECOVERY:
            return "FAST_RECOVERY";
        default:
            return "UNKNOWN_STATE";
    }
}

void init_sender(SenderState *sender) {
    sender->cwnd = 1.0;
    sender->ssthresh = 16.0;
    sender->dup_ack_count = 0;
    sender->in_fast_recovery = 0;
    sender->state = STATE_SLOW_START;
    sender->acked_packets = 0;
    sender->loss_events = 0;
}
