/*
 * Plugin Interface - API Contract
 * 
 * Purpose:
 *   Defines the standard interface that all DPI plugins must implement.
 *   This ensures a consistent way to manage lifecycle (init, cleanup) and
 *   metadata for any protocol or feature plugin.
 * 
 * Usage:
 *   1. Define a struct instance of `dpi_plugin_t`.
 *   2. Implement `init` and `cleanup` functions.
 *   3. Register the plugin with the core registry.
 */

#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#include "../core/dpi_types.h"

/**
 * dpi_plugin_t
 * 
 * Description:
 *   Structure representing a DPI plugin.
 */
typedef struct dpi_plugin {
    const char *name;           // Plugin name (e.g., "apb", "axi")
    const char *version;        // Plugin version string
    plugin_status_t status;     // Current lifecycle status
    
    // Lifecycle callbacks
    int (*init)(void);          // Called during dpi_init_python()
    void (*cleanup)(void);      // Called during dpi_finalize_python()
    
    // Plugin-specific data
    void *private_data;         // Opaque pointer for plugin internal state
} dpi_plugin_t;

// Helper macro for defining a plugin instance
#define DEFINE_PLUGIN(plugin_name, plugin_version) \
    dpi_plugin_t plugin_name##_plugin = { \
        .name = #plugin_name, \
        .version = plugin_version, \
        .status = PLUGIN_UNINITIALIZED, \
        .init = plugin_name##_init, \
        .cleanup = plugin_name##_cleanup, \
        .private_data = NULL \
    }

#endif // PLUGIN_INTERFACE_H
