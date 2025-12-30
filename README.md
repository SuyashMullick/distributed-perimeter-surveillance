# Distributed Fiber-Like Sensor Surveillance System (Simulated, Non-Weapon)

Target: Demonstrate **Advanced systems engineering** and **production-style C++** (interfaces, determinism, V-model verification).

---

## 0. Non-Claims and Boundaries (Must appear verbatim in README + SRS)

This project is a **simulation** of a perimeter surveillance subsystem inspired by distributed sensing concepts. It does **not** claim operational performance.

**Out of scope (explicit):**

- Weapon integration, targeting, or fire control
- Classified sensing methods or performance
- Real deployment, safety certification, or cyber hardening beyond basic hygiene

The project’s purpose is to demonstrate **requirements engineering, interface control, verification, traceability, and disciplined C++ implementation**.

---

## High-Level Architecture

The system consists of four distinct processes interconnected via ZeroMQ over local TCP, with HTTP REST interfaces for UI:

1. `sensor_node` (multiple instances generating Poisson events)
2. `network_emulator` (relaying and introducing latency/jitter/loss)
3. `central_processor` (aggregating and rule-based classifying)
4. `operator_ui` (HTTP server reading local central state)

### Getting Started

#### Prerequisites
- A C++20 compiler
- CMake $\ge$ 3.24
- `vcpkg` available in your path or automatically bootstrapped
- Linux (Ubuntu for CI guarantees)

#### Building
Configure and build using CMake Presets:

```bash
cmake --preset release
cmake --build --preset release
```

#### Running Tests
```bash
ctest --preset release
```

#### Running the System
See run scripts or documentation in `docs/` for execution topologies.
