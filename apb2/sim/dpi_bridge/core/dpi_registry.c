#include "dpi_registry.h"
#include "../plugins/plugin_interface.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 4

dpi_registry_t* dpi_registry_create(void) {
    dpi_registry_t *registry = (dpi_registry_t*)malloc(sizeof(dpi_registry_t));
    if (registry == NULL) {
        DPI_LOG_ERROR("Failed to allocate registry");
        return NULL;
    }

    registry->capacity = INITIAL_CAPACITY;
    registry->count = 0;
    registry->plugins = (dpi_plugin_t**)malloc(sizeof(dpi_plugin_t*) * registry->capacity);
    
    if (registry->plugins == NULL) {
        DPI_LOG_ERROR("Failed to allocate plugin array");
        free(registry);
        return NULL;
    }

    DPI_LOG_INFO("Registry created");
    return registry;
}

void dpi_registry_destroy(dpi_registry_t *registry) {
    if (registry == NULL) {
        return;
    }

    free(registry->plugins);
    free(registry);
    DPI_LOG_INFO("Registry destroyed");
}

int dpi_registry_add_plugin(dpi_registry_t *registry, dpi_plugin_t *plugin) {
    if (registry == NULL || plugin == NULL) {
        DPI_LOG_ERROR("Invalid registry or plugin");
        return DPI_ERROR;
    }

    // Check if we need to resize
    if (registry->count >= registry->capacity) {
        int new_capacity = registry->capacity * 2;
        dpi_plugin_t **new_plugins = (dpi_plugin_t**)realloc(
            registry->plugins, 
            sizeof(dpi_plugin_t*) * new_capacity
        );
        
        if (new_plugins == NULL) {
            DPI_LOG_ERROR("Failed to resize plugin array");
            return DPI_ERROR;
        }
        
        registry->plugins = new_plugins;
        registry->capacity = new_capacity;
    }

    registry->plugins[registry->count] = plugin;
    registry->count++;
    
    DPI_LOG_INFO("Registered plugin: %s", plugin->name);
    return DPI_SUCCESS;
}

dpi_plugin_t* dpi_registry_get_plugin(dpi_registry_t *registry, const char *name) {
    if (registry == NULL || name == NULL) {
        return NULL;
    }

    for (int i = 0; i < registry->count; i++) {
        if (strcmp(registry->plugins[i]->name, name) == 0) {
            return registry->plugins[i];
        }
    }

    DPI_LOG_ERROR("Plugin not found: %s", name);
    return NULL;
}

int dpi_registry_init_all(dpi_registry_t *registry) {
    if (registry == NULL) {
        return DPI_ERROR;
    }

    DPI_LOG_INFO("Initializing %d plugins", registry->count);
    
    for (int i = 0; i < registry->count; i++) {
        dpi_plugin_t *plugin = registry->plugins[i];
        if (plugin->init != NULL) {
            if (plugin->init() != DPI_SUCCESS) {
                DPI_LOG_ERROR("Failed to initialize plugin: %s", plugin->name);
                return DPI_ERROR;
            }
        }
    }

    return DPI_SUCCESS;
}

void dpi_registry_cleanup_all(dpi_registry_t *registry) {
    if (registry == NULL) {
        return;
    }

    DPI_LOG_INFO("Cleaning up %d plugins", registry->count);
    
    for (int i = 0; i < registry->count; i++) {
        dpi_plugin_t *plugin = registry->plugins[i];
        if (plugin->cleanup != NULL) {
            plugin->cleanup();
        }
    }
}
