#include "dpi_core.h"
#include <stdio.h>

static int python_initialized = 0;

int dpi_core_init_python(void) {
    if (python_initialized) {
        DPI_LOG_INFO("Python already initialized");
        return DPI_SUCCESS;
    }

    Py_Initialize();
    
    // Add current directory and common paths to sys.path
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('.')");
    PyRun_SimpleString("sys.path.append('./sim')");
    PyRun_SimpleString("sys.path.append('./dpi_bridge/plugins')");
    
    python_initialized = 1;
    DPI_LOG_INFO("Python initialized successfully");
    return DPI_SUCCESS;
}

void dpi_core_finalize_python(void) {
    if (!python_initialized) {
        return;
    }
    
    Py_Finalize();
    python_initialized = 0;
    DPI_LOG_INFO("Python finalized");
}

PyObject* dpi_core_load_module(const char *module_name, const char *search_path) {
    if (!python_initialized) {
        DPI_LOG_ERROR("Python not initialized");
        return NULL;
    }

    // Add search path if provided
    if (search_path != NULL) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "sys.path.append('%s')", search_path);
        PyRun_SimpleString(cmd);
    }

    PyObject *module = PyImport_ImportModule(module_name);
    if (module == NULL) {
        PyErr_Print();
        DPI_LOG_ERROR("Failed to load module: %s", module_name);
        return NULL;
    }

    DPI_LOG_INFO("Loaded module: %s", module_name);
    return module;
}

PyObject* dpi_core_get_function(PyObject *module, const char *func_name) {
    if (module == NULL) {
        DPI_LOG_ERROR("Module is NULL");
        return NULL;
    }

    PyObject *func = PyObject_GetAttrString(module, func_name);
    if (func == NULL || !PyCallable_Check(func)) {
        if (PyErr_Occurred()) {
            PyErr_Print();
        }
        DPI_LOG_ERROR("Cannot find function: %s", func_name);
        return NULL;
    }

    DPI_LOG_DEBUG("Retrieved function: %s", func_name);
    return func;
}

PyObject* dpi_core_call_function(PyObject *func, PyObject *args) {
    if (func == NULL) {
        DPI_LOG_ERROR("Function is NULL");
        return NULL;
    }

    PyObject *result = PyObject_CallObject(func, args);
    if (result == NULL) {
        PyErr_Print();
        DPI_LOG_ERROR("Function call failed");
    }

    return result;
}
