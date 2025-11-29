/*
 * DPI Bridge - The "Bridge Manager"
 * 
 * FOR SYSTEMVERILOG ENGINEERS:
 * ---------------------------
 * Think of this file as the "Top Level Module" of the C side.
 * Just as you have a `top.sv` that instantiates your DUT and Testbench,
 * this file initializes the entire Python environment.
 * 
 * What it does:
 * 1. `dpi_init_python()`: 
 *    - Starts the embedded Python interpreter (like powering on a CPU).
 *    - Sets up the "Registry" (a list of active plugins).
 *    - Calls the `init()` function of every plugin (APB, Generic, etc.).
 *    - This MUST be called in your SV `initial` block or `end_of_elaboration_phase`.
 * 
 * 2. `dpi_finalize_python()`:
 *    - Shuts everything down cleanly.
 *    - Ensures all Python files are closed and memory is freed.
 *    - This MUST be called in your SV `final` block or `extract_phase`.
 */

#include "dpi_bridge/core/dpi_core.h"
#include "dpi_bridge/core/dpi_registry.h"
#include "dpi_bridge/plugins/apb/apb_plugin.h"
#include "dpi_bridge/plugins/generic/generic_plugin.h"
#include "svdpi.h"

// Global registry to track all active plugins
static dpi_registry_t *g_registry = NULL;

/**
 * dpi_init_python()
 * 
 * Description:
 *   Initializes the Python interpreter and all registered DPI plugins.
 *   This is the first function called by SystemVerilog.
 * 
 * Returns:
 *   0 on success, 1 on failure.
 */
int dpi_init_python() {
    // Initialize Python interpreter
    if (dpi_core_init_python() != DPI_SUCCESS) {
        return 1;
    }

    // Create plugin registry
    g_registry = dpi_registry_create();
    if (g_registry == NULL) {
        dpi_core_finalize_python();
        return 1;
    }

    // Initialize APB plugin
    // TODO: In future, iterate through a list of plugins to initialize dynamically
    if (apb_init() != DPI_SUCCESS) {
        dpi_registry_destroy(g_registry);
        dpi_core_finalize_python();
        return 1;
    }

    // Initialize Generic plugin
    if (generic_init() != DPI_SUCCESS) {
        dpi_registry_destroy(g_registry);
        dpi_core_finalize_python();
        return 1;
    }

    DPI_LOG_INFO("DPI Bridge initialized successfully");
    return 0;
}

/**
 * dpi_finalize_python()
 * 
 * Description:
 *   Cleans up all resources: finalizes plugins, destroys registry, and
 *   shuts down the Python interpreter. Called at end of simulation.
 */
void dpi_finalize_python() {
    // Cleanup APB plugin
    apb_cleanup();
    generic_cleanup();

    // Cleanup registry
    if (g_registry != NULL) {
        dpi_registry_cleanup_all(g_registry);
        dpi_registry_destroy(g_registry);
        g_registry = NULL;
    }

    // Finalize Python
    dpi_core_finalize_python();
    
    DPI_LOG_INFO("DPI Bridge finalized");
}
