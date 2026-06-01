/*
 * Transaction Generation Package
 * 
 * SystemVerilog wrapper for transaction generation DPI functions.
 */
package transaction_pkg;

    // Import DPI-C functions for transaction generation
    import "DPI-C" context function string dpi_get_next_transaction(input string protocol);
    import "DPI-C" context function void dpi_send_response(input string protocol, input string response_json);
    import "DPI-C" context function void dpi_setup_apb_test();

endpackage
