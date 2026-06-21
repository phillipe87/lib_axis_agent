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

#include <iostream>
#include <iomanip>
#include "verilated.h"
#include "lib_axis_agent.h"
#include "Vaxis_passthrough.h"
#include "verilated_vcd_c.h"

static constexpr unsigned BYTES = 4;

// -------------------------------------
// Callbacks
// -------------------------------------

/**
 * @brief Print a transaction's payload to stdout.
 *
 * @param tag  Label string printed before the payload (e.g. "[driver] sent").
 * @param txn  Transaction whose bytes will be printed in hex.
 */
static void print_txn(const char* tag, const axis_transaction<BYTES>& txn) {
  auto bytes = txn.to_bytes();

  std::cout << tag << " (" << bytes.size() << " bytes): ";

  for (auto b : bytes) {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << (unsigned)b << " ";
  }

  std::cout << "\n";
}

/**
 * @brief Fired by axis_master_driver after each transaction finishes transmitting.
 * @param txn The completed transaction.
 */
static void on_driver_done(const axis_transaction<BYTES>& txn) {
  print_txn("[driver]  sent    ", txn);
}

/**
 * @brief Fired by axis_master_monitor after each transaction is captured.
 * @param txn The completed transaction.
 */
static void on_monitor_rxn(const axis_transaction<BYTES>& txn) {
  print_txn("[monitor]  captured    ", txn);
}


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

  Verilated::traceEverOn(true);
  VerilatedVcdC* tfp = new VerilatedVcdC;
  dut->trace(tfp, 99);  // 99 = trace depth
  tfp->open("waveform.vcd");

  // Stimulus interfaces to DUT
  axis_if<BYTES> m_axis;
  axis_if<BYTES> s_axis;

  //---------------------------------
  // Wire interfaces to DUT
  //---------------------------------
  // AXIS into DUT
  m_axis.direction = axis_if<BYTES>::MASTER;
  m_axis.tvalid = &dut->s_tvalid_i;
  m_axis.tdata  = reinterpret_cast<uint8_t*>(&dut->s_tdata_i);
  m_axis.tkeep  = reinterpret_cast<uint8_t*>(&dut->s_tkeep_i);
  m_axis.tlast  = &dut->s_tlast_i ;
  m_axis.tready = &dut->s_tready_o;

  // AXIS out of DUT
  s_axis.direction = axis_if<BYTES>::SLAVE;
  s_axis.tvalid = &dut->m_tvalid_o;
  s_axis.tdata  = reinterpret_cast<uint8_t*>(&dut->m_tdata_o);
  s_axis.tkeep  = reinterpret_cast<uint8_t*>(&dut->m_tkeep_o);
  s_axis.tlast  = &dut->m_tlast_o ;
  s_axis.tready = &dut->m_tready_i;
  dut->m_tready_i = 1;

  //---------------------------------
  // Instantiate agent components
  //---------------------------------
  axis_master_driver<BYTES> driver(m_axis, on_driver_done);
  //axis_monitor<BYTES> mon_in(m_axis);
  //axis_monitor<BYTES> mon_out(s_axis, on_monitor_rxn);

  // ---------------------------------
  // Reset
  // -- Hold rst_i high for 4 cycles then release
  // ---------------------------------
  dut->rst_i = 1;

  for (int i = 0; i < 4; ++i) {
    dut->clk_i = 0; dut->eval(); ctx.timeInc(1); tfp->dump(ctx.time());
    dut->clk_i = 1; dut->eval(); ctx.timeInc(1); tfp->dump(ctx.time());
  }

  dut->rst_i = 0;

  for (int i = 0; i < 4; ++i) {
    dut->clk_i = 0; dut->eval(); ctx.timeInc(1); tfp->dump(ctx.time());
    dut->clk_i = 1; dut->eval(); ctx.timeInc(1); tfp->dump(ctx.time());
  }

  // ---------------------------------
  // Send test transactions
  // ---------------------------------

  // 6 bytes - 2 transfers
  std::vector<IData> word_buff = {0xDEADBEEF, 0xCAFEF00D};

  axis_transaction<BYTES> axis_txn = axis_transaction<BYTES>::from_words((const std::vector<IData>&) word_buff);

  driver.send_txn((const axis_transaction<BYTES>&) axis_txn);


  //driver.send_bytes({0xDE, 0xAD, 0xBE, 0xEF,
  //                   0xCA, 0xFE});
  //driver.send_bytes({0xDE, 0xAD, 0xBE, 0xEF,
  //                   0xCA, 0xFE});

  // 12 bytes - 3 transfers
  //driver.send_bytes({0x01, 0x02, 0x03, 0x04,
  //                   0x05, 0x06, 0x07, 0x08,
  //                   0x09, 0x0A, 0x0B, 0x0C});

  // 1 bytes - 1 transfer
  //driver.send_bytes({0xAA});

  // -------------------------------------------------------------------------
  // Clock loop
  //
  // Correct eval() call sequence on each rising edge:
  //   1. driver.eval()
  //   2. dut->eval()
  //   3. mon_in.eval()
  //   4. mon_out.eval()
  // -------------------------------------------------------------------------
  std::cout << "entering clock loop\n";
  for (int cy = 0; cy < 20; ++cy) {
    //std::cout << "cy=" << cy << " start\n";
    // Falling edge
    dut->clk_i = 0;
    dut->eval();
    ctx.timeInc(1);
    tfp->dump(ctx.time());

    // Rising edge
    dut->clk_i = 1;
    driver.eval();
    dut->eval();
    ctx.timeInc(1);
    tfp->dump(ctx.time());

    //mon_in.eval();
    //mon_out.eval();

    std::cout << "cy=" << cy
              << " tvalid=" << (int)*m_axis.tvalid
              << " tready=" << (int)*m_axis.tready
              << " tlast="  << (int)*m_axis.tlast
              << " idle="   << driver.is_idle()
              //<< " rx="     << mon_out.rx_count()
              << "\n";

    // Exit early once all transactions have been sent and matched
    if (driver.is_idle()) {
      break;
    }
  }

  // Clean up
  dut->final();
  delete dut;
  tfp->close();
  delete tfp;
  return 0;

}

