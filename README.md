
# AutoMicroKernel - Automotive-Grade Safety Microkernel

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![AUTOSAR](https://img.shields.io/badge/AUTOSAR-CP_R22.11-green.svg)](https://www.autosar.org/)
[![Safety](https://img.shields.io/badge/ISO_26262-ASIL_B%2FC-red.svg)](docs/safety/)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)](scripts/ci/)

## Project Overview

**Automotive-grade real-time microkernel operating system** for intelligent vehicles, deeply integrated with TinyML inference capabilities:

- **AUTOSAR Compliant**: Classic Platform OS specification
- **Functional Safety**: ISO 26262 ASIL-B/C certified
- **TinyML Integration**: TensorFlow Lite Micro + CMSIS-NN acceleration
- **Multi-Architecture Support**: ARM Cortex-M/A, RISC-V
- **Real-time Performance**: Microsecond-level scheduling latency

## Design Philosophy
- **Complete Separation of Mechanism and Policy**: The microkernel provides only the most basic scheduling, IPC, memory management, and interrupt mechanisms. All policies (file system, network protocols, device management) run in user-space services.
- **Everything is IPC**: All cross-process communication within the system is completed through efficient and secure inter-process communication primitives, achieving natural fault isolation.
- **Deep TinyML Integration**: Integrated lightweight AI inference, providing full-stack support from model management, real-time scheduling to dedicated memory optimization.
- **Message-Driven, Service-Oriented Architecture**: Based on the kernel's 'one-to-many IPC message routing' mechanism, user-space services communicate through 'publish-subscribe' patterns for decoupled communication, achieving high modularity, fault isolation, and scalability.

## Directory Structure

```
autoMLOS/
├── kernel/                # Microkernel core (mechanism only, <10K LOC)
│   ├── core/              # Four core mechanisms
│   │   ├── scheduler.c    # Deterministic real-time scheduler
│   │   ├── ipc.c          # IPC primitives and built-in message routing
│   │   ├── memory.c       # Virtual and physical memory management
│   │   ├── interrupt.c    # Interrupt takeover and user-space notification framework
│   │   └── syscall.c      # System call entry point
│   └── arch/              # CPU architecture abstraction
├── udrivers/              # User-space device drivers (independent processes)
│   ├── can/               # CAN bus driver
│   ├── camera/            # Camera driver
│   ├── spi/, i2c/, uart/  # Standard bus drivers           
│   ├── watchdog/          # Hardware watchdog driver
│   └── power/             # Power management driver
├── lib/                   # Stateless functional libraries (testable independently)
│   ├── libsyscall/        # System call encapsulation library 
│   ├── logging/           # Logging client library (supports L0-L3 levels)
│   ├── protocol/          # Protocol stack: CAN, UDS, SOME/IP
│   ├── ai/                # TinyML inference engine library
│   ├── diagnostics/       # Diagnostic protocol library (DTC, DEM)
│   └── safety/            # Safety framework (error reporting + recovery strategy templates)
├── services/              # User-space system services (policy and resource management)
│   ├── device_manager/    # Core: Device unified abstraction and message routing hub
│   ├── diagnostic_server/ # UDS/DoIP diagnostic service
│   ├── ai_server/         # AI inference task service
│   ├── logger/            # Log collection, storage, and forwarding service
│   ├── system_monitor/    # System health monitoring and recovery execution service
│   ├── power_manager/     # Power policy service
│   ├── filesystem/        # File system service (user-space, not kernel)
│   └── network/           # Network protocol stack service
├── apps/                  # Applications
│   ├── vehicle_ctrl/      # Vehicle control application
│   └── hmi/               # Human-machine interaction application
├── platform/              # Hardware platform adaptation
│   ├── bsp/               # Board support package
│   └── config/            # System configuration
├── tests/                 # Test framework
├── docs/                  # Documentation
├── tools/                 # Development tools
└── scripts/               # Build scripts
```

## Quick Start

### 1. Environment Preparation

### 2. Build Project

### 3. System Configuration

## ML Scheduler and Testing

## Documentation Index

- [Architecture Design](docs/architecture.md) - System architecture and module interaction description
- [API Reference](docs/api/kernel_api.md) - Complete API documentation
- [Porting Guide](docs/porting_guide.md) - New platform porting
- [Diagnostic Protocol](docs/diagnostic.md) - UDS/DoIP diagnostic protocol implementation
- [Error Handling](docs/safety.md) - Error reporting and fault management guide
- [Performance Test Report](docs/test/test_report.md) - Unit/integration testing
- [Adapter Build](docs/adapter_build.md) - Adaptation layer build and running examples
- [Logging System](docs/logger.md) - Unified logging interface and backend description

## Contribution Guidelines

Welcome to submit Issues and Pull Requests! Please follow:

1. **Code Style**: MISRA C:2012 + AUTOSAR C++14
2. **Testing Requirements**: New features must include unit tests

## License

## Contact Information