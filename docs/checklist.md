# CSE320 Requirement Checklist (Video Excluded)

Source reference: `CSE320_Congestion_Control_Assignment.pdf`

## Core implementation requirements

- [x] Single executable simulates transmission over TCP-like congestion logic  
  Evidence: `Makefile`, `tcp_sim`
- [x] Sender-side `cwnd` behavior modeled  
  Evidence: `src/tcp_state.h`, `src/tcp_state.c`, `src/tcp_reno.c`, `src/tcp_tahoe.c`
- [x] Packet loss events are configurable  
  Evidence: `scenarios/*.txt`, `src/simulator.c:load_events`
- [x] Timeout and triple-duplicate ACK scenarios demonstrated  
  Evidence: `results/reno_timeout.txt`, `results/reno_triple_dup.txt`, `results/tahoe_timeout.txt`, `results/tahoe_triple_dup.txt`
- [x] Step-by-step congestion window evolution printed  
  Evidence: `src/simulator.c:print_simulation_log`, scenario result files in `results/`

## Algorithm scope requirements

- [x] Team-assigned algorithm implemented (`Reno`)  
  Evidence: `README.md` (ID calculation + mapping), `src/tcp_reno.c`
- [x] Comparative algorithm included (`Tahoe`)  
  Evidence: `src/tcp_tahoe.c`, dual-algorithm run commands in `README.md`
- [ ] NewReno partial ACK behavior  
  Not in current scope for this team assignment result

## Topology / forwarding requirements

- [x] Six-node topology supported (`A..F`)  
  Evidence: `topology/A.conf` ... `topology/F.conf`
- [x] Forwarding demonstrations for assignment paths  
  Evidence: `results/topology_A_to_D.txt`, `results/topology_F_to_E.txt`
- [x] Node process mode for six terminals  
  Evidence: `src/simulator.c:run_node_process`, `results/node_mode_transcript.txt`

## Quality gates added by this plan

- [x] Regression command exists and validates baseline outputs  
  Evidence: `Makefile:test`, `scripts/run_regression_tests.sh`
- [x] Scenario parser enforces positive and strictly increasing steps  
  Evidence: `src/simulator.c:load_events`
- [x] CLI parsing is order-independent for optional flags  
  Evidence: `src/main.c`
