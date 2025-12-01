class apb_monitor extends uvm_monitor;
  `uvm_component_utils(apb_monitor)

  virtual apb_if apb_intf;
  
  uvm_analysis_port#(apb_xtn) monitor_ap;

  function new(string name, uvm_component parent);
    super.new(name, parent);
    monitor_ap = new("monitor_ap", this);
  endfunction

  function void build_phase(uvm_phase phase);
    super.build_phase(phase);
  endfunction

  function void connect_phase(uvm_phase phase);
    super.connect_phase(phase);
  endfunction

  task run_phase(uvm_phase phase);
    apb_xtn txn;
    
    forever begin
      // Wait for valid APB transaction (PENABLE && PREADY)
      @(posedge apb_intf.PCLK);
      
      if (apb_intf.PENABLE && apb_intf.PREADY) begin
        txn = apb_xtn::type_id::create("txn");
        
        // Capture transaction details
        txn.apb_address = apb_intf.PADDR;
        txn.apb_strobe  = apb_intf.PSTRB;
        
        if (apb_intf.PWRITE) begin
          // Write transaction
          txn.apb_rd_wr   = apb_xtn::APB_WRITE;
          txn.apb_wr_data = apb_intf.PWDATA;
          `logging(evApb_Transaction, UVM_HIGH, 
                   $sformatf("WRITE addr=0x%0h data=0x%0h", txn.apb_address, txn.apb_wr_data))
        end else begin
          // Read transaction
          txn.apb_rd_wr   = apb_xtn::APB_READ;
          txn.apb_rd_data = apb_intf.PRDATA;
          `logging(evApb_Transaction, UVM_HIGH, 
                   $sformatf("READ addr=0x%0h data=0x%0h", txn.apb_address, txn.apb_rd_data))
        end
        
        // Send transaction to analysis port
        monitor_ap.write(txn);
      end
    end
  endtask

endclass: apb_monitor
