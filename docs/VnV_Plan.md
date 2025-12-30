# Verification and Validation Plan

## 1. Overview
Validation uses a mixture of Test automation (Catch2 CTest pipelines) and procedural demonstration checks.

## 2. Verification Methods

| Requirement | ID | Method | Test Case / Procedure | Pass Criteria |
|-------------|----|--------|-----------------------|---------------|
| Latency ≤ 250ms (p95) | SR-001 | Test | TC-LAT-001 | Latency metrics derived from parsed logs confirm ≤ 250.0 value. |
| Fault Tolerance (≤ 5% drop)| SR-002 | Test | TC-FT-001 | Force kill 2 processes; validate resulting alert tracking vs emission ratios. |
| Logging Completeness | SR-003 | Test | TC-LOG-001 | Validate generated `alerts.jsonl` structure programmatically. |
| Diagnostics No-Interrupt | SR-004 | Demonstration | TC-DIAG-001 | Manual inspection and UI ping under heavy load. |
| Determinism | SR-005 | Test | TC-DET-001 | Verify md5/byte equivalence across successive config-identical runs. |

## 3. Toolset
* `ctest` with Catch2 integrations over C++20 built test executables. Validate raw logs generated recursively in `test` blocks.
