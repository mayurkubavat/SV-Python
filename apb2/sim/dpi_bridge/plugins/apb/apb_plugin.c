/*
 * APB Plugin - The "Custom Hardware" Approach
 * 
 * FOR SYSTEMVERILOG ENGINEERS:
 * ---------------------------
 * This is an example of a "Protocol Specific" plugin.
 * Unlike the Generic plugin, this one is hardcoded for APB.
 * 
 * How it works:
 * 1. `dpi_get_transaction(...)`:
 *    - SV calls this to ask "What should I do next?".
 *    - C calls Python `get_transaction()`.
 *    - Python returns a tuple: `(is_write, addr, data)`.
 *    - C unpacks this tuple and puts values into the `int*` output arguments.
 * 
 * 2. `dpi_send_read_data(...)`:
 *    - SV calls this after a read completes.
 *    - C packs the data into a Python integer and calls `send_read_data()`.
 * 
 * When to use this style?
 * - High Performance: Passing raw integers is faster than parsing strings.
 * - Complex C Logic: If you need to do heavy computation in C before Python sees it.
 * - Legacy Code: If you are integrating with existing C models.
 * 
 * For most DV tasks, prefer the Generic Plugin.
 */

#include "apb_plugin.h"
#include "../plugin_interface.h"
#include "../../core/dpi_core.h"
#include <stdio.h>

// APB Plugin private data
typedef struct {
    PyObject *module;
    PyObject *func_get_transaction;
    PyObject *func_send_read_data;
} apb_plugin_data_t;

static apb_plugin_data_t apb_data = {NULL, NULL, NULL};

/**
 * apb_init()
 * 
 * Description:
 *   Initializes the APB plugin.
 *   Loads `apb_driver` module and retrieves driver functions.
 * 
 * Returns:
 *   DPI_SUCCESS or DPI_ERROR
 */
int apb_init(void) {
    DPI_LOG_INFO("Initializing APB plugin");
    
    // Load APB Python driver module from tests directory
    apb_data.module = dpi_core_load_module("apb_driver", "./tests");
    if (apb_data.module == NULL) {
        DPI_LOG_ERROR("Failed to load apb_driver module from tests/");
        return DPI_ERROR;
    }

    // Get Python functions
    apb_data.func_get_transaction = dpi_core_get_function(apb_data.module, "get_transaction");
    if (apb_data.func_get_transaction == NULL) {
        return DPI_ERROR;
    }

    apb_data.func_send_read_data = dpi_core_get_function(apb_data.module, "send_read_data");
    if (apb_data.func_send_read_data == NULL) {
        return DPI_ERROR;
    }

    DPI_LOG_INFO("APB plugin initialized successfully");
    return DPI_SUCCESS;
}

/**
 * apb_cleanup()
 * 
 * Description:
 *   Releases Python references.
 */
void apb_cleanup(void) {
    DPI_LOG_INFO("Cleaning up APB plugin");
    
    Py_XDECREF(apb_data.func_get_transaction);
    Py_XDECREF(apb_data.func_send_read_data);
    Py_XDECREF(apb_data.module);
    
    apb_data.func_get_transaction = NULL;
    apb_data.func_send_read_data = NULL;
    apb_data.module = NULL;
}

/**
 * dpi_get_transaction()
 * 
 * Description:
 *   Called by SV driver to fetch the next transaction.
 *   Converts Python tuple -> C integers -> SV output arguments.
 * 
 * Args:
 *   time: Current simulation time
 *   is_write, addr, data: Output pointers for transaction details
 * 
 * Returns:
 *   1 if transaction available, 0 if none.
 */
int dpi_get_transaction(dpi_time_t time, int *is_write, int *addr, int *data) {
    PyObject *pArgs, *pValue;

    if (apb_data.func_get_transaction == NULL) {
        DPI_LOG_ERROR("APB plugin not initialized");
        return 0;
    }

    // Create arguments tuple (time)
    pArgs = PyTuple_New(1);
    PyTuple_SetItem(pArgs, 0, PyLong_FromLongLong(time));

    // Call Python function
    pValue = dpi_core_call_function(apb_data.func_get_transaction, pArgs);
    Py_DECREF(pArgs);

    if (pValue != NULL) {
        if (pValue == Py_None) {
            Py_DECREF(pValue);
            return 0; // No more transactions
        }

        // Expected tuple: (is_write, addr, data)
        if (!PyTuple_Check(pValue) || PyTuple_Size(pValue) != 3) {
            DPI_LOG_ERROR("Invalid return value from get_transaction");
            Py_DECREF(pValue);
            return 0;
        }

        *is_write = (int)PyLong_AsLong(PyTuple_GetItem(pValue, 0));
        *addr = (int)PyLong_AsLong(PyTuple_GetItem(pValue, 1));
        *data = (int)PyLong_AsLong(PyTuple_GetItem(pValue, 2));

        Py_DECREF(pValue);
        return 1; // Valid transaction
    }

    return 0;
}

/**
 * dpi_send_read_data()
 * 
 * Description:
 *   Called by SV driver to return read data to Python.
 * 
 * Args:
 *   time: Current simulation time
 *   data: Read data value
 */
void dpi_send_read_data(dpi_time_t time, int data) {
    PyObject *pArgs, *pValue;

    if (apb_data.func_send_read_data == NULL) {
        DPI_LOG_ERROR("APB plugin not initialized");
        return;
    }

    // Create arguments tuple (time, data)
    pArgs = PyTuple_New(2);
    PyTuple_SetItem(pArgs, 0, PyLong_FromLongLong(time));
    PyTuple_SetItem(pArgs, 1, PyLong_FromLong(data));

    // Call Python function
    pValue = dpi_core_call_function(apb_data.func_send_read_data, pArgs);
    Py_DECREF(pArgs);

    if (pValue != NULL) {
        Py_DECREF(pValue);
    }
}
