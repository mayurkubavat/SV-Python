#include "dpi_bridge/core/dpi_core.h"
#include "dpi_bridge/core/dpi_registry.h"
#include "dpi_bridge/plugins/apb/apb_plugin.h"
#include "svdpi.h"

// Global registry
static dpi_registry_t *g_registry = NULL;

// Initialize DPI bridge and all plugins
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
    if (apb_init() != DPI_SUCCESS) {
        dpi_registry_destroy(g_registry);
        dpi_core_finalize_python();
        return 1;
    }

    DPI_LOG_INFO("DPI Bridge initialized successfully");
    return 0;
}

// Finalize DPI bridge and cleanup all plugins
void dpi_finalize_python() {
    // Cleanup APB plugin
    apb_cleanup();

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
