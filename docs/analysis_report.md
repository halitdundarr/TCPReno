# Experimental Analysis - Reno vs Tahoe

## Test Setup
- Executable: `tcp_sim`
- Algorithms: `TCP Reno`, `TCP Tahoe`
- Scenario files:
  - `scenarios/timeout_case.txt`
  - `scenarios/triple_dup_case.txt`
  - `scenarios/mixed_case.txt`

## Reproducibility Commands
Run the following to regenerate the result files:

```bash
make clean && make
./tcp_sim --algo reno --scenario scenarios/timeout_case.txt > results/reno_timeout.txt
./tcp_sim --algo reno --scenario scenarios/triple_dup_case.txt > results/reno_triple_dup.txt
./tcp_sim --algo reno --scenario scenarios/mixed_case.txt > results/reno_mixed.txt
./tcp_sim --algo tahoe --scenario scenarios/timeout_case.txt > results/tahoe_timeout.txt
./tcp_sim --algo tahoe --scenario scenarios/triple_dup_case.txt > results/tahoe_triple_dup.txt
./tcp_sim --algo tahoe --scenario scenarios/mixed_case.txt > results/tahoe_mixed.txt
./tcp_sim --topology-demo A D hello_from_A_to_D > results/topology_A_to_D.txt
./tcp_sim --topology-demo F E hello_from_F_to_E > results/topology_F_to_E.txt
```

## Metric Definitions
- `avg cwnd`: arithmetic mean of per-step `cwnd` values in the simulation log.
- `max cwnd`: maximum `cwnd` observed over all steps.
- `throughput`: `acked_packets / total_steps` from simulation state.
- `recovery_steps`: `(recovery_end_step - recovery_start_step)` where recovery start is first loss-related event and end is first step where `cwnd >= ssthresh`; `N/A` if not reached.

## Metric Summary

### Timeout Case
- Reno: avg cwnd `3.66`, max cwnd `6.00`, throughput `0.93`, recovery steps `2`
- Tahoe: avg cwnd `3.66`, max cwnd `6.00`, throughput `0.93`, recovery steps `2`
- Interpretation: both behave similarly for pure timeout losses in this setup.

### Triple-Duplicate ACK Case
- Reno: avg cwnd `4.17`, max cwnd `6.00`, throughput `0.62`, recovery steps `2`
- Tahoe: avg cwnd `3.45`, max cwnd `6.00`, throughput `0.62`, recovery steps `4`
- Interpretation: Reno recovers faster because it enters fast recovery instead of full restart.

### Mixed Case
- Reno: avg cwnd `3.26`, max cwnd `5.50`, throughput `0.61`, recovery steps `2`
- Tahoe: avg cwnd `2.76`, max cwnd `5.00`, throughput `0.61`, recovery steps `4`
- Interpretation: Reno maintains a larger congestion window under combined losses.

## Topology Validation
- `A -> D` output:
  - `[A] Destination D, next hop B`
  - `[B] Forwarding message from A to D, next hop D`
  - `[D] Received message from A: hello_from_A_to_D`
- `F -> E` output:
  - `[F] Destination E, next hop A`
  - `[A] Forwarding message from F to E, next hop B`
  - `[B] Forwarding message from F to E, next hop E`
  - `[E] Received message from F: hello_from_F_to_E`

## Conclusion
`TCP Reno` (mandatory algorithm for this group) provides faster recovery than Tahoe in duplicate-ACK loss scenarios while preserving stable behavior in timeout cases.
