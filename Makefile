CC = cc
CFLAGS = -std=c11 -Wall -Wextra -pedantic -O2
SRC = src/main.c src/simulator.c src/tcp_state.c src/tcp_reno.c src/tcp_tahoe.c
OUT = tcp_sim

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)

test: $(OUT)
	sh scripts/run_regression_tests.sh

.PHONY: all clean test
