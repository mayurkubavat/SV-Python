package apb_env_pkg;
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

  `include "apb_xtn.svh"
  `include "reset_xtn.svh"

  `include "apb_requester_config.svh"
  `include "apb_completer_config.svh"

  `include "apb_env_config.svh"
  `include "apb_seqr.svh"
  `include "reset_seqr.svh"

  `include "apb_requester_seqs.svh"
  `include "apb_python_seq.sv"
  `include "apb_requester_driver.svh"
  `include "apb_requester_monitor.svh"
  `include "apb_requester.svh"

  `include "apb_completer_driver.svh"
  `include "apb_completer_monitor.svh"
  `include "apb_completer.svh"

  `include "reset_seq.svh"
  `include "reset_driver.svh"
  `include "reset_agent.svh"
  `include "apb_subscriber.svh"

  `include "apb_env.svh"

endpackage: apb_env_pkg
