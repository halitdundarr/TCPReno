# CSE320 TCP Congestion Control Assignment

This project simulates TCP congestion control in C and compares `TCP Reno` and `TCP Tahoe`.

## Team
- `20230808620`
- `20230808615`
- `20230808617`

## Assigned Algorithm Proof
- Formula from assignment: `(student1 + student2 + student3) % 3`
- Calculation: `(20230808620 + 20230808615 + 20230808617) % 3 = 1`
- Mapping: `0=Tahoe, 1=Reno, 2=NewReno`
- Mandatory algorithm for this team: `TCP Reno`

## Build
```bash
make
```

## Run Congestion Control Scenarios
```bash
./tcp_sim --algo reno --scenario scenarios/timeout_case.txt
./tcp_sim --algo reno --scenario scenarios/triple_dup_case.txt
./tcp_sim --algo tahoe --scenario scenarios/timeout_case.txt
./tcp_sim --algo tahoe --scenario scenarios/triple_dup_case.txt
# optional flag ordering is flexible
./tcp_sim --topology-dir topology --algo reno --scenario scenarios/mixed_case.txt
./tcp_sim --algo tahoe --scenario scenarios/mixed_case.txt --topology-dir topology
```

## Run Topology Demonstrations
```bash
./tcp_sim --topology-demo A D hello_from_A_to_D
./tcp_sim --topology-demo F E hello_from_F_to_E
```

## Run 6-Node Process Mode (separate terminals)
Assignment-style entrypoint:
```bash
./node A.conf
./node B.conf
./node C.conf
./node D.conf
./node E.conf
./node F.conf
```

Equivalent simulator mode:
```bash
./tcp_sim --node A
./tcp_sim --node B
./tcp_sim --node C
./tcp_sim --node D
./tcp_sim --node E
./tcp_sim --node F
```

Then type commands in node terminals:
```text
send D hello_from_A_to_D
send E hello_from_F_to_E
```

`./node A.conf` resolves `A.conf` from `topology/A.conf` when run from the repo root, and also accepts
explicit paths such as `./node topology/A.conf`.

Each node process reads its UDP port from the `PORT <number>` line inside the corresponding
`.conf` file and forwards packets according to the routing entries in that same file.

## Implemented Requirements
- Single executable simulation (`tcp_sim`)
- Separate assignment-style node executable (`node`)
- Sender-side `cwnd` and `ssthresh` modeling
- Configurable packet events through scenario files
- Timeout and triple-duplicate ACK demonstrations
- Step-by-step `cwnd` evolution output
- Six-node topology forwarding demonstration (A..F)

## Validation / Regression
Run deterministic regression checks against the stored evidence outputs:
```bash
make test
```

What `make test` validates:
- Reno timeout and triple-duplicate ACK outputs
- Tahoe timeout and triple-duplicate ACK outputs
- Negative case for invalid algorithm
- Negative case for invalid event type in scenario

## Limitations
- Implemented algorithms are `TCP Reno` and `TCP Tahoe` only (no `NewReno` behavior yet).
- Topology mode assumes fixed node set `A..F`.
- Scenario grammar is line-based and supports only: `ACK_NEW`, `ACK_DUP`, `TIMEOUT`.
- Scenario steps must be positive and strictly increasing.

## Known Assumptions
- Congestion window is modeled in MSS units and printed with two decimal precision.
- Forwarding uses static routing tables from `topology/*.conf` without dynamic route updates.
- Each topology file must define its node port with a `PORT <number>` line.
- Regression output files in `results/` are the baseline evidence for reproducible behavior.

## Requirement to Evidence Map
- **Single executable:** `Makefile` builds only `tcp_sim`
- **Core sender structures:** `src/tcp_state.h`, `src/tcp_state.c`
- **Reno/Tahoe behavior:** `src/tcp_reno.c`, `src/tcp_tahoe.c`
- **Configurable loss + scenarios:** `scenarios/timeout_case.txt`, `scenarios/triple_dup_case.txt`, `scenarios/mixed_case.txt`
- **Step-by-step cwnd logs:** run `./tcp_sim --algo reno --scenario scenarios/mixed_case.txt`
- **Timeout and triple-dup demonstrations:** `results/reno_timeout.txt`, `results/reno_triple_dup.txt`, `results/tahoe_timeout.txt`, `results/tahoe_triple_dup.txt`
- **Topology outputs (sample expectations):** `results/topology_A_to_D.txt`, `results/topology_F_to_E.txt`
- **6-terminal process mode evidence:** `results/node_mode_transcript.txt`

## Project Structure
- `src/` source code
- `scenarios/` event input files
- `topology/` routing configuration files
- `results/` generated experiment outputs
- `docs/plan/` team execution plans
- `docs/analysis_report.md` Reno vs Tahoe comparison
- `docs/demo_script.md` spoken demo flow
