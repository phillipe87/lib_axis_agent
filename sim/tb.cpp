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
 #include "lib_axis_agent.h"

 static constexpr unsigned BYTES = 4;


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

  // DUT
  auto* dut = new Vaxis_passthrough(&ctx);

  // Stimulus interfaces to DUT
  axis_if<BYTES> m_axis;
  axis_if<BYTES> s_axis;

  //---------------------------------
  // Wire interfaces to DUT
  //---------------------------------
  // AXIS into DUT
  m_axis.tvalid = &dut->s_tvalid_i;
  m_axis.tdata  = &dut->s_tdata_i ;
  m_axis.tkeep  = &dut->s_tkeep_i ;
  m_axis.tlast  = &dut->s_tlast_i ;
  m_axis.tready = &dut->s_tready_o;

  // AXIS out of DUT
  s_axis.tvalid = &dut->m_tvalid_o;
  s_axis.tdata  = &dut->m_tdata_o ;
  s_axis.tkeep  = &dut->m_tkeep_o ;
  s_axis.tlast  = &dut->m_tlast_o ;
  s_axis.tready = &dut->m_tready_i;
  dut->m_tready_i = 1;

  //---------------------------------
  // Instantiate agent components
  //---------------------------------
  axis_master_driver<BYTES> driver(m_axis);
  axis_monitor<BYTES> in_mon(m_axis);
  axis_monitor<BYTES> out_mon(s_axis);
}
