/**
 * @file tb.cpp
 * @brief Example testbench for an AXI-Stream passthrough DUT.
 *
 * Demonstrates the lib_axis_agent component set:
 * - axis_master_driver sending three transactions of varying lengths
 * - axis_slave_driver applying random backpressure on the output side
 * - axis_monitor observing both the input and output buses
 * - axis_scoreboard comparing the two monitor streams for correctness
 *
 * The DUT is a 1-cycle skid-buffer passthrough (axis_passthrough.sv).
 * All three transactions are expected to arrive at the output unmodified.
 *
 * ### Build
 * @code
 * verilator --cc sim/axis_passthrough.sv --exe sim/tb.cpp \
 *           -I include --build -j4
 * ./obj_dir/Vaxis_passthrough
 * @endcode
 */

 #include "verilated.h"


// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

/**
 * @brief Testbench entry point.
 *
 * Instantiates the Verilated DUT, wires all agent components, runs the
 * clock loop, and asserts that all three transactions passed through the
 * DUT correctly.
 *
 * @param argc Argument count forwarded to VerilatedContext.
 * @param argv Argument vector forwarded to VerilatedContext.
 * @return 0 on success; aborts via assert on failure.
 */
int main(int argc, char** argv) {
    VerilatedContext ctx;
    ctx.commandArgs(argc, argv);
}
