module top;
  `include "uvm_macros.svh"

  import uvm_pkg::*;
  import apb_test_pkg::*;


  // APB Clock
  bit clock;
  parameter CYCLE = 10;

  // APB Interface
  apb_if apb_intf(clock);

  always begin
    clock = ~clock;
    #CYCLE;
  end

  initial begin
    uvm_config_db#(virtual apb_if)::set(null, "*", "apb_if", apb_intf);
    run_test();
  end

  // APB Protocol Assertions - Bind assertion interface to APB interface
  bind apb_if apb_assertions_if apb_assert_if (
    .PCLK     (PCLK),
    .PRESETn  (PRESETn),
    .PADDR    (PADDR),
    .PWDATA   (PWDATA),
    .PRDATA   (PRDATA),
    .PPROT    (PPROT),
    .PSTRB    (PSTRB),
    .PWRITE   (PWRITE),
    .PENABLE  (PENABLE),
    .PREADY   (PREADY),
    .PSLVERR  (PSLVERR)
  );

endmodule: top

