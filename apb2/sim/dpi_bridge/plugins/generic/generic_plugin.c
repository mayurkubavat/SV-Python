/*
 * Generic Plugin - The "Universal Translator"
 * 
 * FOR SYSTEMVERILOG ENGINEERS:
 * ---------------------------
 * This is the recommended way to send data to Python.
 * Instead of writing a new C function for every new transaction type (AXI, UART, PCIe...),
 * we use UVM's built-in `sprint()` to convert the object to a string, and send that.
 * 
 * How it works:
 * 1. SV Side: `dpi_send_object("my_tag", my_obj.sprint(printer));`
 *    - "my_tag": A string ID so Python knows what this object is (e.g., "apb_xtn").
 *    - sprint(): Converts the entire object (fields, arrays) into a text format.
 * 
 * 2. C Side (This File):
 *    - Simply passes the two strings ("tag" and "object_str") to Python.
 *    - It calls `receive_object(tag, object_str)` in `object_receiver.py`.
 * 
 * 3. Python Side:
 *    - The `receive_object` function looks at the "tag".
 *    - It uses a regex parser (like `apb_parser.py`) to convert the string back to a Python dict.
 * 
 * Why use this?
 * - You NEVER have to recompile this C code again.
 * - To add AXI support, you just write a Python parser.
 */

#include "generic_plugin.h"
#include "../../core/dpi_core.h"
#include <stdio.h>

// Generic Plugin private data
typedef struct {
    PyObject *module;
    PyObject *func_receive_object;
} generic_plugin_data_t;

static generic_plugin_data_t generic_data = {NULL, NULL};

/**
 * generic_init()
 * 
 * Description:
 *   Initializes the generic plugin.
 *   Loads the `object_receiver` Python module and retrieves the `receive_object` function.
 * 
 * Returns:
 *   DPI_SUCCESS or DPI_ERROR
 */
int generic_init(void) {
    DPI_LOG_INFO("Initializing Generic plugin");
    
    // Load object receiver Python module from plugins/generic/parsers
    // This module acts as the central dispatcher for all generic objects
    generic_data.module = dpi_core_load_module("object_receiver", "./dpi_bridge/plugins/generic/parsers");
    if (generic_data.module == NULL) {
        DPI_LOG_ERROR("Failed to load object_receiver module from dpi_bridge/plugins/generic/parsers/");
        return DPI_ERROR;
    }

    // Get Python function
    generic_data.func_receive_object = dpi_core_get_function(generic_data.module, "receive_object");
    if (generic_data.func_receive_object == NULL) {
        return DPI_ERROR;
    }

    DPI_LOG_INFO("Generic plugin initialized successfully");
    return DPI_SUCCESS;
}

/**
 * generic_cleanup()
 * 
 * Description:
 *   Releases Python references held by the plugin.
 */
void generic_cleanup(void) {
    DPI_LOG_INFO("Cleaning up Generic plugin");
    
    Py_XDECREF(generic_data.func_receive_object);
    Py_XDECREF(generic_data.module);
    
    generic_data.func_receive_object = NULL;
    generic_data.module = NULL;
}

/**
 * dpi_send_object()
 * 
 * Description:
 *   DPI-C exported function called from SystemVerilog.
 *   Sends a serialized object string to Python.
 * 
 * Args:
 *   tag: Identifier string (e.g., "apb_xtn", "axi_txn") used by Python to select parser.
 *   object_str: The string representation of the object (e.g., from uvm_object::sprint()).
 */
void dpi_send_object(const char* tag, const char* object_str) {
    if (generic_data.func_receive_object == NULL) {
        DPI_LOG_ERROR("Generic plugin not initialized");
        return;
    }

    // Create arguments tuple (tag, object_str)
    PyObject *pArgs = PyTuple_New(2);
    PyTuple_SetItem(pArgs, 0, PyUnicode_FromString(tag));
    PyTuple_SetItem(pArgs, 1, PyUnicode_FromString(object_str));

    // Call Python function
    PyObject *pValue = dpi_core_call_function(generic_data.func_receive_object, pArgs);
    Py_DECREF(pArgs);

    if (pValue != NULL) {
        Py_DECREF(pValue);
    }
}
