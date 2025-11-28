#include <Python.h>
#include <stdio.h>
#include "svdpi.h"

// Global Python objects
PyObject *pModule, *pFuncGet, *pFuncSend;

// Initialize Python and import the driver module
int dpi_init_python() {
    Py_Initialize();
    
    // Add current directory to sys.path
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('.')");
    PyRun_SimpleString("sys.path.append('./sim')"); // In case running from root

    pModule = PyImport_ImportModule("apb_driver");
    if (pModule == NULL) {
        PyErr_Print();
        fprintf(stderr, "Failed to load apb_driver module\n");
        return 1;
    }

    pFuncGet = PyObject_GetAttrString(pModule, "get_transaction");
    if (!pFuncGet || !PyCallable_Check(pFuncGet)) {
        if (PyErr_Occurred()) PyErr_Print();
        fprintf(stderr, "Cannot find function get_transaction\n");
        return 1;
    }

    pFuncSend = PyObject_GetAttrString(pModule, "send_read_data");
    if (!pFuncSend || !PyCallable_Check(pFuncSend)) {
        if (PyErr_Occurred()) PyErr_Print();
        fprintf(stderr, "Cannot find function send_read_data\n");
        return 1;
    }

    printf("[C-Bridge] Python initialized and module loaded.\n");
    return 0;
}

// Clean up Python
void dpi_finalize_python() {
    Py_XDECREF(pFuncGet);
    Py_XDECREF(pFuncSend);
    Py_XDECREF(pModule);
    Py_Finalize();
    printf("[C-Bridge] Python finalized.\n");
}

// Get transaction from Python
// Returns: 1 if valid transaction, 0 if no more transactions
// Outputs: is_write, addr, data
int dpi_get_transaction(int *is_write, int *addr, int *data) {
    PyObject *pValue;

    if (!pFuncGet) return 0;

    pValue = PyObject_CallObject(pFuncGet, NULL);

    if (pValue != NULL) {
        if (pValue == Py_None) {
            Py_DECREF(pValue);
            return 0; // No more transactions
        }

        // Expected tuple: (is_write, addr, data)
        if (!PyTuple_Check(pValue) || PyTuple_Size(pValue) != 3) {
            fprintf(stderr, "Returned value is not a tuple of size 3\n");
            Py_DECREF(pValue);
            return 0;
        }

        *is_write = (int)PyLong_AsLong(PyTuple_GetItem(pValue, 0));
        *addr = (int)PyLong_AsLong(PyTuple_GetItem(pValue, 1));
        *data = (int)PyLong_AsLong(PyTuple_GetItem(pValue, 2));

        Py_DECREF(pValue);
        return 1;
    } else {
        PyErr_Print();
        return 0;
    }
}

// Send read data back to Python
void dpi_send_read_data(int data) {
    PyObject *pArgs, *pValue;

    if (!pFuncSend) return;

    pArgs = PyTuple_New(1);
    PyTuple_SetItem(pArgs, 0, PyLong_FromLong(data));

    pValue = PyObject_CallObject(pFuncSend, pArgs);
    Py_DECREF(pArgs);

    if (pValue != NULL) {
        Py_DECREF(pValue);
    } else {
        PyErr_Print();
    }
}
