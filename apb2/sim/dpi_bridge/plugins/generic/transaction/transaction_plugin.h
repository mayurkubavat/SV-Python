/*
 * Transaction Generation Plugin - Header
 */

#ifndef TRANSACTION_PLUGIN_H
#define TRANSACTION_PLUGIN_H

#include "../../../core/dpi_types.h"

// Plugin lifecycle
int transaction_init(void);
void transaction_cleanup(void);

// DPI-C functions exported to SystemVerilog
const char* dpi_get_next_transaction(const char* protocol);
void dpi_send_response(const char* protocol, const char* response_json);
void dpi_setup_apb_test(void);

#endif // TRANSACTION_PLUGIN_H
