/*
 * DPI Core - The "Engine Room"
 * 
 * FOR SYSTEMVERILOG ENGINEERS:
 * ---------------------------
 * This file handles the low-level translation between C and Python.
 * You generally don't need to touch this unless you are adding deep core features.
 * 
 * Key Concepts:
 * 1. PyObject*: 
 *    - Think of this as a `void*` or a generic handle in SV. 
 *    - In Python, EVERYTHING is an object (integers, strings, functions, classes).
 *    - In C, we hold pointers to these objects using `PyObject*`.
 * 
 * 2. Reference Counting (Py_INCREF/Py_DECREF):
 *    - Python uses garbage collection. When C holds a pointer to a Python object,
 *      we must tell Python "I am using this, don't delete it" (INCREF).
 *    - When we are done, we say "I'm finished" (DECREF).
 *    - If you see `Py_DECREF` in the code, it's just memory management.
 * 
 * 3. sys.path:
 *    - Just like `+incdir+` in Verilog.
 *    - We explicitly add `./sim` and `./plugins` to `sys.path` so Python can `import` your scripts.
 */

#include "dpi_core.h"
#include <stdio.h>

static int python_initialized = 0;

/**
 * dpi_core_init_python()
 * 
 * Description:
 *   Initializes the Python interpreter.
 *   Sets up the Python path (sys.path) to include the simulation directory
 *   and plugin directories, ensuring Python modules can be found.
 * 
 * Returns:
 *   DPI_SUCCESS or DPI_ERROR
 */
int dpi_core_init_python(void) {
    if (python_initialized) {
        DPI_LOG_INFO("Python already initialized");
        return DPI_SUCCESS;
    }

    Py_Initialize();
    
    // Add current directory and common paths to sys.path
    // This allows Python scripts to be located in ./sim, ./sim/tests, etc.
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('.')");
    PyRun_SimpleString("sys.path.append('./sim')");
    PyRun_SimpleString("sys.path.append('./dpi_bridge/plugins')");
    
    python_initialized = 1;
    DPI_LOG_INFO("Python initialized successfully");
    return DPI_SUCCESS;
}

/**
 * dpi_core_finalize_python()
 * 
 * Description:
 *   Shuts down the Python interpreter and releases resources.
 */
void dpi_core_finalize_python(void) {
    if (!python_initialized) {
        return;
    }
    
    Py_Finalize();
    python_initialized = 0;
    DPI_LOG_INFO("Python finalized");
}

/**
 * dpi_core_load_module()
 * 
 * Description:
 *   Loads a Python module by name.
 *   Optionally appends a search path to sys.path before loading.
 * 
 * Args:
 *   module_name: Name of the Python file (without .py)
 *   search_path: Optional directory path to add to sys.path
 * 
 * Returns:
 *   PyObject* pointer to the loaded module, or NULL on failure.
 */
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
        PyErr_Print(); // Print Python traceback to stdout/log
        DPI_LOG_ERROR("Failed to load module: %s", module_name);
        return NULL;
    }

    DPI_LOG_INFO("Loaded module: %s", module_name);
    return module;
}

/**
 * dpi_core_get_function()
 * 
 * Description:
 *   Retrieves a callable function object from a loaded module.
 * 
 * Args:
 *   module: Pointer to the loaded module object
 *   func_name: Name of the function to retrieve
 * 
 * Returns:
 *   PyObject* pointer to the function, or NULL if not found.
 */
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

/**
 * dpi_core_call_function()
 * 
 * Description:
 *   Calls a Python function with the provided arguments.
 * 
 * Args:
 *   func: Pointer to the function object
 *   args: PyObject* tuple containing arguments
 * 
 * Returns:
 *   PyObject* result of the function call, or NULL on failure.
 */
PyObject* dpi_core_call_function(PyObject *func, PyObject *args) {
    if (func == NULL) {
        DPI_LOG_ERROR("Function is NULL");
        return NULL;
    }

    PyObject *result = PyObject_CallObject(func, args);
    if (result == NULL) {
        PyErr_Print(); // Critical: Print traceback if Python code raises exception
        DPI_LOG_ERROR("Function call failed");
    }

    return result;
}
