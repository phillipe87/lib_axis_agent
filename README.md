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
CMakeLists.txt
```

## Requirements

- C++17 compiler
- [Verilator](https://www.veripool.org/verilator/) ≥ 5.0
- CMake ≥ 3.18
