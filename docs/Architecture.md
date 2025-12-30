# Architecture

## 1. Component Responsibilities

* **Sensor Nodes**: Simulate physical sensing environments asynchronously representing stochastic phenomena in configurable patterns.
* **Network Emulator**: Enacts the bridge topologies representing unreliable network channels via deliberate delays, jitter implementations and message discard algorithms.
* **Central Processor**: Determines threat severities based on predetermined energy bounds via standard event rule engines, ensuring states are stored persistently locally.
* **Operator UI**: Interacts with the Operator via standard HTTP protocols to securely visualize local states decoupled from intense event classification paths.

## 2. Data Flows

`SN -> tcp 7001 -> NE -> tcp 7002 -> CP -> logs <-> UI`

1. Sensor bounds raw data, serializes JSON, invokes `zmq_send`.
2. Network buffers, blocks in priority queuing, applies offsets, fires downstream.
3. Central Processor ingests, validates schema, processes classification rules, drops state to file.
4. Operator UI routinely queries files, parsing tail outputs, resolving REST requests to frontend rendering.

## 3. Fault Handling Model

If a `sensor_node` aborts or loses power, its zeroMQ heartbeats fall off. 
The Central Processor tracks last-seen periods. If `age_s > heartbeat_timeout_s` (defaults ~3s) it updates local memory struct to `FAILED`. UI fetches new JSON and paints the node red.
Overall latency rules are immune to isolated node drops.

## 4. Deterministic Mode

Enabled via CLI config loading. Bypasses real-time limits and replaces real `std::this_thread::sleep_for` inside sensor emission threads with simple for-loop logic matching expected timestamps `dt`.
The network emulator likewise processes queues instantly relying directly on monotonic timestamps provided in payloads, guaranteeing byte-identical log outcomes when identical seed numbers are loaded across identical topology setups.
