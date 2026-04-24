# Demo Video Script

## 1) Introduction (20-30 sec)
- "We implemented TCP congestion control simulation in C for CSE320."
- "Our mandatory algorithm is TCP Reno, and we also implemented TCP Tahoe for comparison."

## 2) Build and Run (30 sec)
- Show:
  - `make`
  - `./tcp_sim --algo reno --scenario scenarios/triple_dup_case.txt`
  - `./tcp_sim --algo tahoe --scenario scenarios/triple_dup_case.txt`

## 3) Congestion Window Evolution (45 sec)
- Highlight printed columns: `step`, `event`, `cwnd`, `ssthresh`, `state`.
- Explain:
  - Slow start growth
  - Congestion avoidance transition
  - Behavior after packet loss

## 4) Timeout Scenario (30 sec)
- Run timeout scenario for at least one algorithm.
- Explain `cwnd` drop and `ssthresh` update.

## 5) Triple-Duplicate ACK Scenario (45 sec)
- Compare Reno and Tahoe:
  - Reno enters fast recovery.
  - Tahoe falls back to `cwnd = 1`.
- Mention measured recovery steps from `docs/analysis_report.md`.

## 6) Topology Demonstration (45 sec)
- Run:
  - `./tcp_sim --topology-demo A D hello_from_A_to_D`
  - `./tcp_sim --topology-demo F E hello_from_F_to_E`
- Show next-hop forwarding lines.

## 7) Closing (15 sec)
- "Reno recovers faster than Tahoe in duplicate ACK loss cases."
- "All required artifacts are included: source code, README, and demo evidence."
