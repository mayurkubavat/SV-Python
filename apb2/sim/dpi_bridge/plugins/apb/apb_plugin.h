#ifndef APB_PLUGIN_H
#define APB_PLUGIN_H

#include "../../core/dpi_types.h"

// APB Plugin initialization and cleanup
int apb_init(void);
void apb_cleanup(void);

// DPI-C exported functions for SystemVerilog
int dpi_get_transaction(dpi_time_t time, int *is_write, int *addr, int *data);
void dpi_send_read_data(dpi_time_t time, int data);

// Plugin registration
void apb_register_plugin(void);

#endif // APB_PLUGIN_H
