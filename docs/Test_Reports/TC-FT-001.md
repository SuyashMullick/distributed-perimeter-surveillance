# Test Report: TC-FT-001

* **Environment**: Ubuntu x64, Catch2 Test Runner
* **Config Used**: `config/system_nominal.json`
* **Test Date**: Automated CI execution

## Results
| Metric | Expected | Actual | Pass/Fail |
|--------|----------|--------|-----------|
| Missed Events Ratio | `<= 0.05` | Verified programmatically | PASS |

**Justification**: Killing two independent node branches correctly isolated failure paths from propagating across ZeroMQ subscriptions. Wait limits on central processor correctly registered drop off and flagged missing.
