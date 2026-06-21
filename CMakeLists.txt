cmake_minimum_required(VERSION 3.18)
project(lib_axis_agent CXX)
set(CMAKE_CXX_STANDARD 17)

# Agent library
add_library(lib_axis_agent STATIC
    src/axis_master_driver.cpp
    src/axis_monitor.cpp
)
target_include_directories(lib_axis_agent PUBLIC include)

target_include_directories(lib_axis_agent PRIVATE include
  /mnt/tools/verilator/v5.038/share/verilator/include
)

# Example sim
option(BUILD_SIM "Build the passthrough example simulation" OFF)
if (BUILD_SIM)
    find_package(verilator HINTS $ENV{VERILATOR_ROOT} REQUIRED)
    add_executable(tb sim/tb.cpp)
    target_link_libraries(tb PRIVATE lib_axis_agent)
    verilate(tb
        SOURCES sim/axis_passthrough.sv
        PREFIX  Vaxis_passthrough
        TRACE
    )
endif()
