# System Requirements Specification (SRS)

## 0. Non-Claims and Boundaries
This project is a **simulation** of a perimeter surveillance subsystem inspired by distributed sensing concepts. It does **not** claim operational performance.

**Out of scope (explicit):**
- Weapon integration, targeting, or fire control
- Classified sensing methods or performance
- Real deployment, safety certification, or cyber hardening beyond basic hygiene

The project’s purpose is to demonstrate **requirements engineering, interface control, verification, traceability, and disciplined C++ implementation**.

## 1. System Context
The Distributed Fiber-Like Sensor Surveillance System (Simulated, Non-Weapon) demonstrates strict control over messaging, deterministic execution constraints, and validation.

## 2. Requirements

### SR-001 Latency
Under nominal network conditions, the system shall generate a `CentralAlert` within **≤ 250 ms (p95)** from disturbance generation time.
- **Rationale**: Strict temporal bounding ensures operators are notified quickly of perimeter threats.

### SR-002 Fault Tolerance
With **10** sensor nodes configured, the system shall continue producing alerts when **2 nodes fail** during execution, with **≤ 5% missed alerts** from remaining nodes over a **10-minute** run.
- **Rationale**: Demonstrates decentralized robustness against unit attrition.

### SR-003 Logging Completeness
The system shall log **100%** of `CentralAlert` messages to disk with required fields (timestamp_utc, source_node_id, event_id, classification, processing_latency_ms).
- **Rationale**: Guaranteed audit paths for post-mission analysis.

### SR-004 Diagnostics Without Interrupt
The operator shall query node health and last-seen heartbeat via the operator UI without interrupting event processing.
- **Rationale**: System visibility must not hinder operational functionality.

### SR-005 Determinism
Given identical configuration and identical random seeds, the system shall produce an identical sequence of `CentralAlert` records (field-by-field identical) for a fixed-duration deterministic test run.
- **Rationale**: Regression scaling, debuggability, and behavior validation.

## 3. Assumptions
- Operating System guarantees local loopback (TCP `127.0.0.1`) latency within reasonable bounds (< 1ms natively).
- Process priority is unhindered by severe CPU choking during regular test runs.
