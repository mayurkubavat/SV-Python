# Modular DPI Bridge Architecture

## Overview

The DPI bridge has been refactored into a modular, plugin-based architecture that allows easy extension for new protocols while maintaining clean separation of concerns.

## Directory Structure

```
sim/
├── dpi_bridge.c                    # Main entry point
├── dpi_bridge/
│   ├── core/
│   │   ├── dpi_types.h             # Common types and macros
│   │   ├── dpi_core.h/c            # Python lifecycle management
│   │   └── dpi_registry.h/c        # Plugin registry
│   └── plugins/
│       ├── plugin_interface.h      # Plugin API contract
│       └── apb/
│           ├── apb_plugin.h/c      # APB protocol plugin
│           └── apb_driver.py       # APB Python driver
```

## Architecture Components

### Core Layer (`dpi_bridge/core/`)

**dpi_types.h**: Common definitions
- Return codes (DPI_SUCCESS, DPI_ERROR)
- Time types (dpi_time_t)
- Logging macros (DPI_LOG_INFO, DPI_LOG_ERROR, DPI_LOG_DEBUG)

**dpi_core.h/c**: Python interpreter management
- `dpi_core_init_python()` - Initialize Python interpreter
- `dpi_core_finalize_python()` - Cleanup Python
- `dpi_core_load_module()` - Load Python modules
- `dpi_core_get_function()` - Retrieve Python functions
- `dpi_core_call_function()` - Call Python functions

**dpi_registry.h/c**: Plugin management
- `dpi_registry_create()` - Create plugin registry
- `dpi_registry_add_plugin()` - Register a plugin
- `dpi_registry_get_plugin()` - Lookup plugin by name
- `dpi_registry_init_all()` - Initialize all plugins
- `dpi_registry_cleanup_all()` - Cleanup all plugins

### Plugin Interface (`dpi_bridge/plugins/plugin_interface.h`)

Standard plugin structure:
```c
typedef struct dpi_plugin {
    const char *name;
    const char *version;
    plugin_status_t status;
    int (*init)(void);
    void (*cleanup)(void);
    void *private_data;
} dpi_plugin_t;
```

### APB Plugin (`dpi_bridge/plugins/apb/`)

APB-specific implementation:
- `apb_init()` - Load APB Python driver
- `apb_cleanup()` - Cleanup APB resources
- `dpi_get_transaction()` - DPI-C function for SV
- `dpi_send_read_data()` - DPI-C function for SV

## Building

Compile all source files together:

```bash
gcc -shared -fPIC -o libdpi_bridge.so \
  dpi_bridge.c \
  dpi_bridge/core/dpi_core.c \
  dpi_bridge/core/dpi_registry.c \
  dpi_bridge/plugins/apb/apb_plugin.c \
  $(python3-config --cflags --ldflags) \
  -I/tools/Xilinx/2025.1/Vivado/data/xsim/include \
  -I.
```

Or use the automated build:
```bash
sim.py --top top --filelist apb_inc_xilinx.f --uvm --test apb_init_test --sv_lib dpi_bridge
```

## Adding a New Plugin

1. **Create plugin directory**: `dpi_bridge/plugins/my_protocol/`

2. **Create plugin header** (`my_protocol_plugin.h`):
```c
#ifndef MY_PROTOCOL_PLUGIN_H
#define MY_PROTOCOL_PLUGIN_H

#include "../../core/dpi_types.h"

int my_protocol_init(void);
void my_protocol_cleanup(void);

// DPI-C functions
int dpi_my_protocol_transaction(...);

#endif
```

3. **Implement plugin** (`my_protocol_plugin.c`):
```c
#include "my_protocol_plugin.h"
#include "../../core/dpi_core.h"

int my_protocol_init(void) {
    // Load Python module
    // Get functions
    return DPI_SUCCESS;
}

void my_protocol_cleanup(void) {
    // Cleanup resources
}
```

4. **Register in dpi_bridge.c**:
```c
#include "dpi_bridge/plugins/my_protocol/my_protocol_plugin.h"

int dpi_init_python() {
    // ... existing code ...
    
    if (my_protocol_init() != DPI_SUCCESS) {
        // handle error
    }
}
```

5. **Add to build**: Include new `.c` file in compilation

## Benefits

- ✅ **Modularity**: Clear separation between core and protocol logic
- ✅ **Extensibility**: Easy to add new protocols (AXI, AHB, etc.)
- ✅ **Reusability**: Core Python management shared across plugins
- ✅ **Maintainability**: Each component has single responsibility
- ✅ **Testability**: Plugins can be tested independently

## Migration Notes

The refactoring maintains full backward compatibility:
- Same DPI-C function signatures
- Same Python driver API
- Same SystemVerilog interface
- Existing tests pass without modification
