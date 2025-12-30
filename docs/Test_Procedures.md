# Test Procedures

## TC-LAT-001
1. **Setup**: Load `system_nominal.json`.
2. **Execute**: Launch `sensor_node`, `network_emulator`, `central_processor` collectively. Wait 15s.
3. **Assert**: Validate `run_logs/alerts.jsonl` contains processing_latency_ms arrays. Calculate p95. Assert `p95 <= 250`.

## TC-FT-001
1. **Setup**: Load `system_nominal.json`.
2. **Execute**: Launch systems. Wait 5s. Emit SIGTERM to two specific `sensor_nodes`.
3. **Execute**: Wait 10s. Evaluate node's local emissions against `central_alerts` records.
4. **Assert**: Mis-matched local UUID counts must be `< 5%` total pool.

## TC-DET-001
1. **Setup**: Load `system_deterministic.json`.
2. **Execute**: Run `central_processor`, `network_emulator`, `sensor_nodes` sequentially utilizing mode `deterministic`. Copy logs to `sandbox_1`.
3. **Execute**: Repeat step 2 exact operations. Copy logs to `sandbox_2`.
4. **Assert**: `sandbox_1/alerts.jsonl` is byte-identical to `sandbox_2/alerts.jsonl`.
