CC = cc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -O2
COMMON_SRC = src/simulator.c src/tcp_state.c src/tcp_reno.c src/tcp_tahoe.c
SIM_SRC = src/main.c $(COMMON_SRC)
NODE_SRC = src/node_main.c $(COMMON_SRC)
OUT = tcp_sim
NODE_OUT = node

all: $(OUT) $(NODE_OUT)

$(OUT): $(SIM_SRC)
	$(CC) $(CFLAGS) $(SIM_SRC) -o $(OUT)

$(NODE_OUT): $(NODE_SRC)
	$(CC) $(CFLAGS) $(NODE_SRC) -o $(NODE_OUT)

clean:
	rm -f $(OUT) $(NODE_OUT)

test: $(OUT) $(NODE_OUT)
	sh scripts/run_regression_tests.sh

.PHONY: all clean test
