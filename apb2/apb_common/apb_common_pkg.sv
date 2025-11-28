package apb_common_pkg;
  
  `include "uvm_macros.svh"
  
  import uvm_pkg::*;

  typedef enum {
    evReset_Assertion,
    evReset_Deassertion,
    evApb_Transaction,
    evApb_ReadRequest
  } event_log_e;

  function string get_event_name(event_log_e e);
    return e.name();
  endfunction

  `include "apb_logging.svh"
  `include "apb_xtn.svh"
  `include "apb_monitor.svh"
  `include "apb_seqr.svh"
  
endpackage: apb_common_pkg
