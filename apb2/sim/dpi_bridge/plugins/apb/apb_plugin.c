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

// Plugin lifecycle
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

void apb_cleanup(void) {
    DPI_LOG_INFO("Cleaning up APB plugin");
    
    Py_XDECREF(apb_data.func_get_transaction);
    Py_XDECREF(apb_data.func_send_read_data);
    Py_XDECREF(apb_data.module);
    
    apb_data.func_get_transaction = NULL;
    apb_data.func_send_read_data = NULL;
    apb_data.module = NULL;
}

// DPI-C function: Get APB transaction from Python
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

// DPI-C function: Send read data to Python
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
