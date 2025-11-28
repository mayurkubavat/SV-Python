#ifndef DPI_TYPES_H
#define DPI_TYPES_H

#include <Python.h>
#include <stdint.h>

// Common return codes
#define DPI_SUCCESS 0
#define DPI_ERROR   1

// Time type for simulation time
typedef int64_t dpi_time_t;

// Plugin status
typedef enum {
    PLUGIN_UNINITIALIZED = 0,
    PLUGIN_INITIALIZED,
    PLUGIN_ACTIVE,
    PLUGIN_ERROR
} plugin_status_t;

// Logging macros
#define DPI_LOG_INFO(fmt, ...) \
    printf("[DPI-INFO] " fmt "\n", ##__VA_ARGS__)

#define DPI_LOG_ERROR(fmt, ...) \
    fprintf(stderr, "[DPI-ERROR] " fmt "\n", ##__VA_ARGS__)

#define DPI_LOG_DEBUG(fmt, ...) \
    printf("[DPI-DEBUG] " fmt "\n", ##__VA_ARGS__)

#endif // DPI_TYPES_H
