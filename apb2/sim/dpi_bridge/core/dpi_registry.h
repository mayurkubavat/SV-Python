#ifndef DPI_REGISTRY_H
#define DPI_REGISTRY_H

#include "dpi_types.h"

// Forward declaration
typedef struct dpi_plugin dpi_plugin_t;

// Plugin registry
typedef struct {
    dpi_plugin_t **plugins;
    int count;
    int capacity;
} dpi_registry_t;

// Registry management
dpi_registry_t* dpi_registry_create(void);
void dpi_registry_destroy(dpi_registry_t *registry);

// Plugin registration
int dpi_registry_add_plugin(dpi_registry_t *registry, dpi_plugin_t *plugin);

// Plugin lookup
dpi_plugin_t* dpi_registry_get_plugin(dpi_registry_t *registry, const char *name);

// Initialize all registered plugins
int dpi_registry_init_all(dpi_registry_t *registry);

// Cleanup all registered plugins
void dpi_registry_cleanup_all(dpi_registry_t *registry);

#endif // DPI_REGISTRY_H
