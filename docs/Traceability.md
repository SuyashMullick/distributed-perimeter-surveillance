# Traceability Matrix

| Requirement ID | Description | Test Case | Evidence Artifact |
|----------------|-------------|-----------|-------------------|
| SR-001 | Latency ≤ 250ms p95 | TC-LAT-001 | `tests/test_latency.cpp` |
| SR-002 | Fault Tolerance ≤ 5% loss on 2 drops | TC-FT-001 | `tests/test_fault_tolerance.cpp` |
| SR-003 | Logging Completeness | TC-LOG-001 | `tests/test_logging.cpp` |
| SR-004 | Diagnostics Without Interrupt | TC-DIAG-001 | Demonstrated continuously. |
| SR-005 | Determinism | TC-DET-001 | `tests/test_determinism.cpp` |
