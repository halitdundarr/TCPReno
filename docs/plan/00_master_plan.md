# CSE320 Master Execution Plan

## Mandatory Algorithm Decision
- Student IDs: `20230808620`, `20230808615`, `20230808617`
- Sum `% 3 = 1` -> mandatory algorithm is `TCP Reno`.
- To satisfy demo requirement, project also implements `TCP Tahoe` for direct comparison.

## Workstream
1. Build C project skeleton and shared data structures.
2. Implement Reno (slow start, avoidance, fast retransmit, fast recovery).
3. Implement Tahoe (timeout + triple-dup fallback to `cwnd = 1`).
4. Add configurable scenarios (timeout, triple-dup, mixed).
5. Add A..F topology configs and forwarding demonstration.
6. Run experiments and collect metrics.
7. Finalize README and demo script.

## Done Criteria
- One executable: `tcp_sim`
- Step-by-step `cwnd` trace printed
- Timeout and triple-duplicate ACK demonstrated
- At least two algorithms executed
- Forwarding outputs match assignment samples
