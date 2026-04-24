#ifndef TCP_ALGORITHMS_H
#define TCP_ALGORITHMS_H

#include "tcp_state.h"

void apply_tahoe(SenderState *sender, EventType event);
void apply_reno(SenderState *sender, EventType event);

#endif
