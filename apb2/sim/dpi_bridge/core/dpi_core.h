#ifndef DPI_CORE_H
#define DPI_CORE_H

#include "dpi_types.h"

// Python interpreter management
int dpi_core_init_python(void);
void dpi_core_finalize_python(void);

// Python module loading
PyObject* dpi_core_load_module(const char *module_name, const char *search_path);

// Python function retrieval
PyObject* dpi_core_get_function(PyObject *module, const char *func_name);

// Utility: Call Python function with arguments
PyObject* dpi_core_call_function(PyObject *func, PyObject *args);

#endif // DPI_CORE_H
