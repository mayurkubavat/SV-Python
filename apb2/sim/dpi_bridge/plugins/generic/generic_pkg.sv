/*
 * Generic Plugin Package
 * 
 * Purpose:
 *   SystemVerilog wrapper for the Generic DPI Plugin.
 *   Exposes the `dpi_send_object` function to SystemVerilog code.
 * 
 * Usage:
 *   import generic_pkg::*;
 *   dpi_send_object("my_tag", my_obj.sprint(printer));
 */
package generic_pkg;

    // Import DPI-C function for sending UVM objects to Python
    // tag: Identifier for the object type (used by Python dispatcher)
    // object_str: Serialized string representation of the object
    import "DPI-C" context function void dpi_send_object(input string tag, input string object_str);

endpackage
