// Module: APB Python Sequence
// Filename: apb_python_seq.sv

import "DPI-C" context function int dpi_init_python();
import "DPI-C" context function void dpi_finalize_python();
import "DPI-C" context function int dpi_get_transaction(input longint time_ps, output int is_write, output int addr, output int data);
import "DPI-C" context function void dpi_send_read_data(input longint time_ps, input int data);

class apb_python_seq extends apb_base_seq;
  `uvm_object_utils(apb_python_seq)

  extern function new(string name = "apb_python_seq");
  extern task body();

endclass: apb_python_seq

function apb_python_seq::new(string name = "apb_python_seq");
  super.new(name);
endfunction

task apb_python_seq::body();
  int is_write;
  int addr;
  int data;
  int valid;

  if (dpi_init_python() != 0) begin
    `uvm_error("APB_PYTHON_SEQ", "Failed to initialize Python")
    return;
  end

  forever begin
    valid = dpi_get_transaction($time, is_write, addr, data);
    if (valid == 0) break;

    req = apb_xtn::type_id::create("req");
    start_item(req);
    
    if (is_write) begin
      req.apb_address = addr;
      req.apb_wr_data = data;
      req.apb_rd_wr = apb_xtn::APB_WRITE;
    end else begin
      req.apb_address = addr;
      req.apb_rd_wr = apb_xtn::APB_READ;
    end
    
    // Randomize other fields if necessary, or just finish
    // assert(req.randomize()); // Might overwrite our values if not careful with constraints
    // Instead of full randomize, we set specific fields. 
    // If there are other required fields, we should handle them.
    // Assuming psel, penable etc are handled by driver.
    
    finish_item(req);

    if (!is_write) begin
      dpi_send_read_data($time, req.apb_rd_data);
    end
  end

  dpi_finalize_python();
endtask
