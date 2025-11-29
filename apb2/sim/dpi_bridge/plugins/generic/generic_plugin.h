#ifndef GENERIC_PLUGIN_H
#define GENERIC_PLUGIN_H

#include "../../core/dpi_types.h"

// Plugin lifecycle
int generic_init(void);
void generic_cleanup(void);

// DPI-C functions
// Send a UVM object string with a tag to Python
void dpi_send_object(const char* tag, const char* object_str);

#endif // GENERIC_PLUGIN_H
