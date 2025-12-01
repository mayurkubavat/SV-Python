class apb_requester_driver extends uvm_driver#(apb_xtn);
  `uvm_component_utils(apb_requester_driver)

  virtual apb_if apb_intf;

  apb_requester_config requester_cfg_h;

  function new(string name, uvm_component parent);
    super.new(name, parent);
  endfunction

  function void build_phase(uvm_phase phase);
    if(!uvm_config_db#(apb_requester_config)::get(this, "", "apb_requester_config", requester_cfg_h)) begin
      `uvm_fatal(get_full_name(), "Cannot get VIF from configuration database!")
    end
    super.build_phase(phase);
  endfunction

  function void connect_phase(uvm_phase phase);
    apb_intf = requester_cfg_h.apb_intf;
    super.connect_phase(phase);
  endfunction

  extern task drive();

  task run_phase(uvm_phase phase);
    fork
      // Detect RESET signals, Disable Driver on detection
      forever begin
        wait(!apb_intf.PRESETn);
        `logging(evReset_Assertion, UVM_MEDIUM, "")
        disable driver;
        apb_intf.PADDR  <= 0;
        apb_intf.PPROT  <= 0;
        apb_intf.PENABLE <= 0;
        apb_intf.PWDATA <= 0;
        apb_intf.PSTRB  <= 0;

        wait(apb_intf.PRESETn);
        `logging(evReset_Deassertion, UVM_MEDIUM, "")
      end

      forever begin
        seq_item_port.get_next_item(req);
        begin: driver
          wait(apb_intf.PRESETn);
          drive();
        end  // driver
        seq_item_port.item_done(req);
      end
    join
  endtask

endclass: apb_requester_driver

task apb_requester_driver::drive();
  `logging(evApb_Transaction, UVM_MEDIUM, $sformatf("addr=%0h data=%0h write=%b", req.apb_address, req.apb_wr_data, req.apb_rd_wr))

  // SETUP Phase - Set all signals
  @(posedge apb_intf.PCLK);
  
  if(req.apb_rd_wr == apb_xtn::APB_READ)
    apb_intf.PWRITE <= 0;
  else begin
    apb_intf.PWRITE <= 1;
    apb_intf.PWDATA <= req.apb_wr_data;
    apb_intf.PSTRB  <= req.apb_strobe;
  end

  apb_intf.PPROT  <= 1;
  apb_intf.PADDR  <= req.apb_address;
  apb_intf.PENABLE <= 0;

  // Wait one cycle for SETUP phase
  @(posedge apb_intf.PCLK);

  // ACCESS Phase - Assert PENABLE
  apb_intf.PENABLE <= 1;

  // Wait for PREADY to be asserted
  @(posedge apb_intf.PCLK);
  while (!apb_intf.PREADY) begin
    @(posedge apb_intf.PCLK);
  end
  
  // Capture read data for read transactions
  if(req.apb_rd_wr == apb_xtn::APB_READ) begin
    req.apb_rd_data = apb_intf.PRDATA;
  end
  
  // Deassert PENABLE (return to IDLE)
  apb_intf.PENABLE <= 0;

  // Wait one more cycle before changing other signals
  @(posedge apb_intf.PCLK);
  apb_intf.PWRITE  <= 0; // Default to Read/Idle
  apb_intf.PADDR   <= 0;

endtask
