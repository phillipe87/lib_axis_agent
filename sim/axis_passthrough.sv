// Minimal 1-cycle skid-buffer passthrough
// Used to smoke-test the agent.
module axis_passthrough #(
  parameter int unsigned W = 32
) (
  input  logic           clk_i,
  input  logic           rst_i,
  // Slave port
  input  logic           s_tvalid_i,
  output logic           s_tready_o,
  input  logic [W-1:0]   s_tdata_i,
  input  logic [W/8-1:0] s_tkeep_i,
  input  logic           s_tlast_i,
  // Master port
  output logic           m_tvalid_o,
  input  logic           m_tready_i,
  output logic [W-1:0]   m_tdata_o,
  output logic [W/8-1:0] m_tkeep_o,
  output logic           m_tlast_o
);

  logic           buf_valid_r;
  logic [W-1:0]   buf_data_r;
  logic [W/8-1:0] buf_keep_r;
  logic           buf_last_r;

  logic s_tready_c;

  // ack incoming transfer when
  // -- buffer is empty - buf_valid=0
  // OR
  // -- buffer gets emptied - m_tready=1
  assign s_tready_c = !buf_valid_r || m_tready_r;

  always_ff @(posedge clk_i) begin
    if (rst_i) begin
      buf_valid_r <= 0;
    end else if ( (s_tvalid_i && s_tready_r) && (m_tready_i || !buf_valid_r) ) begin // assert when buffering transfer while emptying buffer or while buffer is empty
      buf_valid_r <= 1;
    end else if (!(s_tvalid_i && s_tready_r) && m_tready_i) begin // deassert when emptying buffer with no incoming transfer
      buf_valid_r <= 0;
    end
  end

  always_ff @(posedge clk_i) begin
    if ( (s_tvalid_i && s_tready_r) && (m_tready_i || !buf_valid_r) ) begin // buffer transfer while emptying buffer or while buffer is empty
      buf_valid_r <= 1;
      buf_data_r  <= s_tdata_i;
      buf_keep_r  <= s_tkeep_i;
      buf_last_r  <= s_tlast_i;
    end
  end

  assign m_tvalid_o = buf_valid_r;
  assign m_tdata_o  = buf_data_r;
  assign m_tkeep_o  = buf_keep_r;
  assign m_tlast_o  = buf_last_r;
  assign s_tready_o = s_tready_c;
endmodule
