`ifndef APB_LOGGING_SVH
`define APB_LOGGING_SVH

`define logging(event, verbosity, msg) \
  `uvm_info(get_event_name(event), msg, verbosity)

`endif // APB_LOGGING_SVH