/*
 * Transaction Generation Plugin - C Interface
 * 
 * Provides DPI-C functions for generic transaction generation from Python.
 */

#include "transaction_plugin.h"
#include "../../../core/dpi_core.h"
#include <stdio.h>

// Transaction Generation Plugin private data
typedef struct {
    PyObject *module;
    PyObject *func_get_next;
    PyObject *func_send_response;
    PyObject *func_setup_test;
} transaction_plugin_data_t;

static transaction_plugin_data_t transaction_data = {NULL, NULL, NULL, NULL};

/**
 * transaction_init()
 * 
 * Description:
 *   Initializes the transaction generation plugin.
 *   Loads the Python generator module.
 */
int transaction_init(void) {
    DPI_LOG_INFO("Initializing Transaction Generation plugin");
    
    // Load transaction generator module
    transaction_data.module = dpi_core_load_module("transaction_generator", "./dpi_bridge/plugins/generic/transaction");
    if (transaction_data.module == NULL) {
        DPI_LOG_ERROR("Failed to load transaction_generator module");
        return DPI_ERROR;
    }

    // Get Python functions
    transaction_data.func_get_next = dpi_core_get_function(transaction_data.module, "dpi_get_next_transaction");
    if (transaction_data.func_get_next == NULL) {
        return DPI_ERROR;
    }

    transaction_data.func_send_response = dpi_core_get_function(transaction_data.module, "dpi_send_response");
    if (transaction_data.func_send_response == NULL) {
        return DPI_ERROR;
    }

    transaction_data.func_setup_test = dpi_core_get_function(transaction_data.module, "dpi_setup_apb_test");
    if (transaction_data.func_setup_test == NULL) {
        return DPI_ERROR;
    }

    DPI_LOG_INFO("Transaction Generation plugin initialized successfully");
    return DPI_SUCCESS;
}

/**
 * transaction_cleanup()
 * 
 * Description:
 *   Releases Python references.
 */
void transaction_cleanup(void) {
    DPI_LOG_INFO("Cleaning up Transaction Generation plugin");
    
    Py_XDECREF(transaction_data.func_get_next);
    Py_XDECREF(transaction_data.func_send_response);
    Py_XDECREF(transaction_data.func_setup_test);
    Py_XDECREF(transaction_data.module);
    
    transaction_data.func_get_next = NULL;
    transaction_data.func_send_response = NULL;
    transaction_data.func_setup_test = NULL;
    transaction_data.module = NULL;
}

/**
 * dpi_get_next_transaction()
 * 
 * Description:
 *   DPI-C exported function to get next transaction from Python.
 *   
 * Args:
 *   protocol: Protocol name (e.g., "apb", "axi")
 *   
 * Returns:
 *   JSON string with transaction data, or empty string if done
 */
const char* dpi_get_next_transaction(const char* protocol) {
    static char result_buffer[4096];
    
    if (transaction_data.func_get_next == NULL) {
        DPI_LOG_ERROR("Transaction Generation plugin not initialized");
        return "";
    }

    // Create arguments tuple (protocol)
    PyObject *pArgs = PyTuple_New(1);
    PyTuple_SetItem(pArgs, 0, PyUnicode_FromString(protocol));

    // Call Python function
    PyObject *pValue = dpi_core_call_function(transaction_data.func_get_next, pArgs);
    Py_DECREF(pArgs);

    if (pValue != NULL) {
        const char *str = PyUnicode_AsUTF8(pValue);
        if (str != NULL) {
            snprintf(result_buffer, sizeof(result_buffer), "%s", str);
            Py_DECREF(pValue);
            return result_buffer;
        }
        Py_DECREF(pValue);
    }

    return "";
}

/**
 * dpi_send_response()
 * 
 * Description:
 *   DPI-C exported function to send response to Python.
 *   
 * Args:
 *   protocol: Protocol name
 *   response_json: JSON string with response data
 */
void dpi_send_response(const char* protocol, const char* response_json) {
    if (transaction_data.func_send_response == NULL) {
        DPI_LOG_ERROR("Transaction Generation plugin not initialized");
        return;
    }

    // Create arguments tuple (protocol, response_json)
    PyObject *pArgs = PyTuple_New(2);
    PyTuple_SetItem(pArgs, 0, PyUnicode_FromString(protocol));
    PyTuple_SetItem(pArgs, 1, PyUnicode_FromString(response_json));

    // Call Python function
    PyObject *pValue = dpi_core_call_function(transaction_data.func_send_response, pArgs);
    Py_DECREF(pArgs);

    if (pValue != NULL) {
        Py_DECREF(pValue);
    }
}

/**
 * dpi_setup_apb_test()
 * 
 * Description:
 *   DPI-C exported function to setup APB test in Python.
 */
void dpi_setup_apb_test(void) {
    if (transaction_data.func_setup_test == NULL) {
        DPI_LOG_ERROR("Transaction Generation plugin not initialized");
        return;
    }

    // Call Python function (no args)
    PyObject *pValue = dpi_core_call_function(transaction_data.func_setup_test, NULL);

    if (pValue != NULL) {
        Py_DECREF(pValue);
    }
}
