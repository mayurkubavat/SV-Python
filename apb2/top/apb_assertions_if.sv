/*
 * APB Protocol Assertion Interface
 * 
 * Implements comprehensive SVA checks for AMBA APB protocol compliance.
 * This interface is bound to the APB interface for automatic assertion checking.
 */

interface apb_assertions_if(
  input logic        PCLK,
  input logic        PRESETn,
  input logic [31:0] PADDR,
  input logic [31:0] PWDATA,
  input logic [31:0] PRDATA,
  input logic [2:0]  PPROT,
  input logic [3:0]  PSTRB,
  input logic        PWRITE,
  input logic        PENABLE,
  input logic        PREADY,
  input logic        PSLVERR
);

  //=============================================================================
  // Protocol State Definitions
  //=============================================================================
  
  typedef enum {IDLE, SETUP, ACCESS} apb_state_e;
  
  // Derive current state from PENABLE
  apb_state_e current_state;
  always_comb begin
    if (!PRESETn)
      current_state = IDLE;
    else if (!PENABLE)
      current_state = SETUP;
    else
      current_state = ACCESS;
  end

  //=============================================================================
  // Basic Protocol Assertions
  //=============================================================================

  // APB_SETUP_TO_ACCESS: PENABLE must go high after SETUP phase
  property p_setup_to_access;
    @(posedge PCLK) disable iff (!PRESETn)
    (current_state == SETUP) |=> (current_state == ACCESS);
  endproperty
  assert_setup_to_access: assert property(p_setup_to_access)
    else $error("[APB_ASSERTION] SETUP phase must transition to ACCESS phase");

  // APB_PENABLE_DEASSERT: PENABLE must deassert when PREADY is high
  property p_penable_deassert;
    @(posedge PCLK) disable iff (!PRESETn)
    (PENABLE && PREADY) |=> !PENABLE;
  endproperty
  assert_penable_deassert: assert property(p_penable_deassert)
    else $error("[APB_ASSERTION] PENABLE must deassert after PREADY");

  // APB_PENABLE_PULSE: PENABLE must be a single-cycle pulse when PREADY is high
  property p_penable_single_pulse;
    @(posedge PCLK) disable iff (!PRESETn)
    $rose(PENABLE) |-> ##[1:$] (PREADY && PENABLE);
  endproperty
  assert_penable_single_pulse: assert property(p_penable_single_pulse)
    else $error("[APB_ASSERTION] PENABLE must remain high until PREADY");

  //=============================================================================
  // Signal Stability Assertions
  //=============================================================================

  // APB_PADDR_STABLE: Address must remain stable during ACCESS phase
  property p_paddr_stable;
    @(posedge PCLK) disable iff (!PRESETn)
    (PENABLE && !PREADY) |=> $stable(PADDR);
  endproperty
  assert_paddr_stable: assert property(p_paddr_stable)
    else $error("[APB_ASSERTION] PADDR must remain stable during ACCESS phase");

  // APB_PWRITE_STABLE: Write signal must remain stable during ACCESS phase
  property p_pwrite_stable;
    @(posedge PCLK) disable iff (!PRESETn)
    (PENABLE && !PREADY) |=> $stable(PWRITE);
  endproperty
  assert_pwrite_stable: assert property(p_pwrite_stable)
    else $error("[APB_ASSERTION] PWRITE must remain stable during ACCESS phase");

  // APB_PWDATA_STABLE: Write data must remain stable during write ACCESS phase
  property p_pwdata_stable;
    @(posedge PCLK) disable iff (!PRESETn)
    (PENABLE && !PREADY && PWRITE) |=> $stable(PWDATA);
  endproperty
  assert_pwdata_stable: assert property(p_pwdata_stable)
    else $error("[APB_ASSERTION] PWDATA must remain stable during write ACCESS phase");

  // APB_PPROT_STABLE: Protection signal must remain stable during ACCESS phase
  property p_pprot_stable;
    @(posedge PCLK) disable iff (!PRESETn)
    (PENABLE && !PREADY) |=> $stable(PPROT);
  endproperty
  assert_pprot_stable: assert property(p_pprot_stable)
    else $error("[APB_ASSERTION] PPROT must remain stable during ACCESS phase");

  // APB_PSTRB_STABLE: Strobe must remain stable during write ACCESS phase
  property p_pstrb_stable;
    @(posedge PCLK) disable iff (!PRESETn)
    (PENABLE && !PREADY && PWRITE) |=> $stable(PSTRB);
  endproperty
  assert_pstrb_stable: assert property(p_pstrb_stable)
    else $error("[APB_ASSERTION] PSTRB must remain stable during write ACCESS phase");

  //=============================================================================
  // PREADY Behavior Assertions
  //=============================================================================

  // APB_PREADY_EVENTUALLY: PREADY must eventually go high during ACCESS phase
  property p_pready_eventually;
    @(posedge PCLK) disable iff (!PRESETn)
    PENABLE |-> ##[1:16] PREADY;
  endproperty
  assert_pready_eventually: assert property(p_pready_eventually)
    else $error("[APB_ASSERTION] PREADY must assert within 16 cycles during ACCESS phase");

  //=============================================================================
  // Reset Behavior Assertions
  //=============================================================================

  // APB_RESET_PENABLE: PENABLE must be low during reset
  property p_reset_penable;
    @(posedge PCLK)
    !PRESETn |-> !PENABLE;
  endproperty
  assert_reset_penable: assert property(p_reset_penable)
    else $error("[APB_ASSERTION] PENABLE must be low during reset");

  //=============================================================================
  // PSTRB Validity Assertions
  //=============================================================================

  // APB_PSTRB_VALID: PSTRB bits must align with valid byte lanes
  property p_pstrb_valid;
    @(posedge PCLK) disable iff (!PRESETn)
    (PENABLE && PWRITE) |-> (PSTRB inside {4'b0001, 4'b0010, 4'b0100, 4'b1000,
                                            4'b0011, 4'b1100, 4'b1111});
  endproperty
  assert_pstrb_valid: assert property(p_pstrb_valid)
    else $warning("[APB_ASSERTION] PSTRB has unusual pattern (not a common byte enable)");

  //=============================================================================
  // PPROT Validity Assertions
  //=============================================================================

  // APB_PPROT_RANGE: PPROT must be within valid range
  property p_pprot_range;
    @(posedge PCLK) disable iff (!PRESETn)
    PENABLE |-> (PPROT <= 3'b111);
  endproperty
  assert_pprot_range: assert property(p_pprot_range)
    else $error("[APB_ASSERTION] PPROT must be within valid range [0:7]");

  //=============================================================================
  // Transaction Sequencing Assertions
  //=============================================================================

  // APB_NO_BACK_TO_BACK_WITHOUT_IDLE: Transactions should have IDLE between them
  property p_transaction_spacing;
    @(posedge PCLK) disable iff (!PRESETn)
    (PENABLE && PREADY) |=> !PENABLE;
  endproperty
  assert_transaction_spacing: assert property(p_transaction_spacing)
    else $error("[APB_ASSERTION] Must return to IDLE/SETUP after ACCESS completes");

endinterface: apb_assertions_if
