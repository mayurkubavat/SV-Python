# Modular DPI Bridge Architecture

## Overview

The DPI bridge provides a modular, plugin-based architecture for seamless SystemVerilog-Python communication. It enables verification engineers to leverage Python's ecosystem (ML, data analysis, visualization) while maintaining their SystemVerilog testbench.

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
│       ├── apb/                    # APB protocol plugin
│       │   ├── apb_plugin.h/c      # APB-specific DPI functions
│       └── generic/                # Universal object serialization
│           ├── generic_plugin.h/c  # Generic string transport
│           ├── generic_pkg.sv      # SV helper package
│           └── parsers/            # Python parsers (centralized)
│               ├── uvm_parser.py   # Base UVM parser
│               ├── apb_parser.py   # APB-specific parser
│               └── object_receiver.py  # Receiver dispatcher
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

### Generic Plugin (`dpi_bridge/plugins/generic/`) - **RECOMMENDED**

**Purpose**: Universal serialization for any UVM object using `sprint()` with line printer.

**Architecture**:
- **Single plugin** handles all UVM objects (APB, AXI, UART, etc.)
- **Centralized parsers** in `parsers/` subdirectory
- **Protocol-agnostic** string transport layer

**C Plugin**:
- `generic_init()` - Load object receiver module
- `generic_cleanup()` - Cleanup resources
- `dpi_send_object(tag, object_str)` - Send any UVM object string to Python

**SystemVerilog Package** (`generic_pkg.sv`):
```systemverilog
import generic_pkg::*;
dpi_send_object("my_tag", my_obj.sprint(printer));
```

**Python Parsers** (`parsers/`):
- `uvm_parser.py` - Base parser class with field extraction utilities
- `apb_parser.py` - APB transaction parser
- `object_receiver.py` - Dispatcher that routes objects to protocol-specific parsers

**Usage Example**:
```systemverilog
// In your test (e.g., apb_dpi_object_test.svh)
import generic_pkg::*;

class apb_dpi_object_test extends uvm_test;
    virtual task run_phase(uvm_phase phase);
        uvm_line_printer printer = new();
        printer.knobs.reference = 0;
        
        apb_xtn xtn = apb_xtn::type_id::create("xtn");
        xtn.randomize();
        
        // Send to Python - works for ANY UVM object
        dpi_send_object("apb_xtn_uvm", xtn.sprint(printer));
    endtask
endclass
```

**Adding New Protocol Parser**:
1. Create `parsers/my_protocol_parser.py` extending `UVMObjectParser`
2. Update `object_receiver.py` to handle new tag
3. No C code changes or recompilation needed!

**Benefits**:
- ✅ Automatic serialization via UVM `sprint()`
- ✅ No manual field-by-field JSON building
- ✅ Scalable: add new protocols without touching C
- ✅ Centralized: all parsers in one location
- ✅ **Remember to flush stdout**: Use `sys.stdout.flush()` after `print()` in Python

### APB Plugin (`dpi_bridge/plugins/apb/`)

APB-specific implementation for high-performance transaction-level modeling:
- `apb_init()` - Load APB Python driver
- `apb_cleanup()` - Cleanup APB resources
- `dpi_get_transaction()` - DPI-C function for SV
- `dpi_send_read_data()` - DPI-C function for SV

**When to use**: High performance, legacy integration, or complex C-side logic.

## Building

Compile all source files together:

```bash
gcc -shared -fPIC -o libdpi_bridge.so \
  dpi_bridge.c \
  dpi_bridge/core/dpi_core.c \
  dpi_bridge/core/dpi_registry.c \
  dpi_bridge/plugins/apb/apb_plugin.c \
  dpi_bridge/plugins/generic/generic_plugin.c \
  $(python3-config --cflags --ldflags) \
  -I/tools/Xilinx/2025.1/Vivado/data/xsim/include \
  -I.
```

Or use the automated build:
```bash
sim.py --top top --filelist apb_inc_xilinx.f --uvm --test apb_dpi_object_test --sv_lib dpi_bridge
```

## Adding a New Plugin

1. **Create plugin directory**: `dpi_bridge/plugins/my_protocol/`

2. **Create plugin header** (`my_protocol_plugin.h`)

3. **Implement plugin** (`my_protocol_plugin.c`)

4. **Register in dpi_bridge.c**

5. **Add to build**: Include new `.c` file in compilation

## Future Scope: Python-Driven Verification

### 1. Python-Controlled Testbench

**Vision**: Shift test control from SystemVerilog to Python, reducing SV to a thin interface layer.

**Implementation**:
- **Phase Control**: Python calls `run_test()`, `run_phase()` via DPI
- **Sequence Generation**: Python generates stimulus, SV executes
- **Configuration**: Python configures UVM components via DPI

**Benefits**:
- Leverage Python's rich libraries (NumPy, Pandas, Matplotlib)
- Easier debugging with Python IDEs
- Faster iteration (no recompilation for test changes)

**Example**:
```python
# Python test
def run_verification():
    dpi_configure_env({"num_agents": 4})
    for i in range(100):
        txn = generate_random_apb_txn()
        dpi_send_transaction(txn)
    dpi_wait_for_completion()
```

### 2. Machine Learning Integration

**Use Cases**:
- **Coverage Closure**: ML models predict uncovered scenarios
- **Constrained Random**: ML-guided stimulus generation
- **Bug Prediction**: Analyze waveforms/logs to predict bugs

**Implementation**:
```python
# Train model on coverage data
model = train_coverage_model(past_runs)
# Generate targeted stimulus
next_test = model.predict_uncovered_scenario()
dpi_send_test_config(next_test)
```

### 3. Advanced Data Analysis

**Capabilities**:
- Real-time performance monitoring (bandwidth, latency)
- Statistical analysis of transaction patterns
- Automated report generation with charts

**Example**:
```python
import matplotlib.pyplot as plt
import pandas as pd

# Collect data from SV
df = pd.DataFrame(transactions)
df['latency'].plot(kind='hist')
plt.savefig('latency_distribution.png')
```

### 4. Dynamic Plugin Loading

**Vision**: Load plugins from configuration files instead of hardcoding.

**Implementation**:
```json
{
  "plugins": [
    {"name": "apb", "path": "./plugins/apb"},
    {"name": "axi", "path": "./plugins/axi"}
  ]
}
```

### 5. RTL Interaction

**Direct RTL Access**: Use DPI to read/write RTL signals from Python.

**Use Cases**:
- Fault injection
- Power analysis
- Custom assertions

**Example**:
```systemverilog
// SV side
import "DPI-C" function void dpi_inject_fault(string signal_path, bit value);

// Python side
def inject_single_bit_flip():
    dpi_inject_fault("top.dut.fifo.write_ptr[3]", 1)
```

### 6. Co-Simulation with External Tools

**Integration Points**:
- MATLAB/Simulink for DSP verification
- TensorFlow for AI accelerator verification
- Custom C++ models

**Example**:
```python
# Python bridges SV and external tool
sv_data = dpi_get_output()
matlab_result = matlab_engine.process(sv_data)
dpi_send_expected(matlab_result)
```

### 7. Unified Logging and Reporting

**Vision**: Centralize all logs (UVM, Python, C) into a single database.

**Benefits**:
- Searchable logs
- Automated regression analysis
- Web-based dashboards

## Benefits

- ✅ **Modularity**: Clear separation between core and protocol logic
- ✅ **Extensibility**: Easy to add new protocols (AXI, AHB, etc.)
- ✅ **Reusability**: Core Python management shared across plugins
- ✅ **Maintainability**: Each component has single responsibility
- ✅ **Testability**: Plugins can be tested independently
- ✅ **Future-Proof**: Designed for Python-driven verification workflows

## Migration Notes

The refactoring maintains full backward compatibility:
- Same DPI-C function signatures
- Same Python driver API
- Same SystemVerilog interface
- Existing tests pass without modification
