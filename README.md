# lib_axis_agent

A C++ AXI-Stream verification agent for use with Verilator.
It provides a clean, UVM-inspired component model, including transaction, driver, monitor, and scoreboard,
without the UVM runtime overhead.

## Components

|Class            | Role |
|---              |---|
|`axis_if<BYTES>` | Interface — pointer bundle into the Verilated model |

## Directory layout

```
include/
  axis_agent.h (A single header containing all declarations and types)
src/
  axis_master_driver.cpp (Driver component)
  axis_monitor.cpp (Monitor component)
CMakeLists.txt
```

## Build
```bash
cmake --build build --target clean

cmake -B build -DBUILD_SIM=ON -Dverilator_DIR=${VERILATOR_ROOT}

cmake --build build -j4
```

## Requirements

- C++17 compiler
- [Verilator](https://www.veripool.org/verilator/) ≥ 5.0
- CMake ≥ 3.18

## License

Copyright (C) 2025 Phillipe —
released under the [GNU General Public License v3.0](LICENSE).

EDA vendors and anyone distributing derived works must do so under the same
license. As the sole copyright holder, the author retains the right to use
this library in closed-source commercial products.
