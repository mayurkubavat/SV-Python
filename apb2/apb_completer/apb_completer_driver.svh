class apb_completer_driver extends uvm_driver#(apb_xtn);
  `uvm_component_utils(apb_completer_driver)

  virtual apb_if apb_intf;

  apb_completer_config completer_cfg_h;

  function new(string name, uvm_component parent);
    super.new(name, parent);
  endfunction

  function void build_phase(uvm_phase phase);
    if(!uvm_config_db#(apb_completer_config)::get(this, "", "apb_completer_config", completer_cfg_h)) begin
      `uvm_fatal(get_full_name(), "Cannot get VIF from configuration database!")
    end
    super.build_phase(phase);
  endfunction

  function void connect_phase(uvm_phase phase);
    apb_intf = completer_cfg_h.apb_intf;
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
        apb_intf.PREADY  <= 0;
        apb_intf.PSLVERR <= 0;
        apb_intf.PRDATA  <= 0;

        wait(apb_intf.PRESETn);
        `logging(evReset_Deassertion, UVM_MEDIUM, "")
      end

      forever begin: driver
        wait(apb_intf.PRESETn);
        drive();
      end
    join
  endtask

endclass

task apb_completer_driver::drive();
  do
    repeat({$random} % 10) @(apb_intf.sdrv_cb);
  while(!apb_intf.sdrv_cb.PENABLE);

  apb_intf.sdrv_cb.PREADY <= 1;

  if(!apb_intf.sdrv_cb.PWRITE) begin
    `logging(evApb_ReadRequest, UVM_MEDIUM, $sformatf("addr=%0h", apb_intf.sdrv_cb.PADDR))
    // Read Operation
    apb_intf.sdrv_cb.PRDATA <= $random;
  end

  @(apb_intf.sdrv_cb);
  apb_intf.sdrv_cb.PREADY <= 0;

endtask
