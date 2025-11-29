package apb_test_pkg;

  `include "uvm_macros.svh"
  
  import uvm_pkg::*;
  import apb_common_pkg::*;
  import apb_env_pkg::*;

    // Import DPI-C functions
    import "DPI-C" context function int dpi_init_python();
    import "DPI-C" context function void dpi_send_object(input string tag, input string object_str);

  `include "apb_base_test.svh"
  `include "apb_init_test.svh"
  `include "apb_reset_test.svh"
  `include "apb_dpi_object_test.svh"

endpackage: apb_test_pkg
