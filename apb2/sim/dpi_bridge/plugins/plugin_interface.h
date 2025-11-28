#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#include "../core/dpi_types.h"

// Plugin interface structure
typedef struct dpi_plugin {
    const char *name;           // Plugin name (e.g., "apb", "axi")
    const char *version;        // Plugin version
    plugin_status_t status;     // Current status
    
    // Lifecycle callbacks
    int (*init)(void);          // Initialize plugin
    void (*cleanup)(void);      // Cleanup plugin resources
    
    // Plugin-specific data
    void *private_data;         // Plugin-specific context
} dpi_plugin_t;

// Helper macro for plugin definition
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
