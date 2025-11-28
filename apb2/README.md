# APB2 UVM Testbench with Python-C-SV DPI Bridge

UVM Verification IP for AMBA APB v2.0 bus protocol with integrated Python-to-SystemVerilog DPI bridge for transaction-level testing.

## Overview

This testbench demonstrates a complete SV-C-Python integration using DPI-C (Direct Programming Interface) to enable Python scripts to drive APB transactions directly into the SystemVerilog simulation. This allows for:
- Python-based test stimulus generation
- Bidirectional communication between Python and SystemVerilog
- Read/write transaction control from Python
- Read data feedback to Python

## Directory Structure

```
apb2/
├── apb_completer/       # APB completer (slave) agent
├── apb_env/             # UVM environment and transaction definitions
├── apb_requester/       # APB requester (master) agent
│   └── apb_python_seq.sv    # Python-driven sequence using DPI-C
├── apb_test/            # UVM test cases
│   └── apb_init_test.svh    # Test using Python sequence
├── reset_agent/         # Reset agent
├── sim/                 # Simulation files
│   ├── dpi_bridge/          # Modular DPI bridge
│   │   ├── core/            # Core Python management
│   │   └── plugins/apb/     # APB protocol plugin
│   ├── tests/               # Python test files
│   │   ├── apb_base.py      # Base classes and utilities
│   │   ├── apb_driver.py    # DPI interface
│   │   ├── apb_basic_test.py   # Basic test
│   │   ├── apb_burst_test.py   # Burst test
│   │   └── apb_random_test.py  # Random test
│   ├── apb_inc_xilinx.f     # Xilinx file list
│   └── sim.py               # Simulation script
├── top/                 # Top-level testbench
└── README.md
```

## Architecture

### DPI Bridge Components

**Modular Architecture:**

1. **Core Layer (`sim/dpi_bridge/core/`)**
   - Python interpreter lifecycle management
   - Module loading and function retrieval
   - Plugin registry system

2. **APB Plugin (`sim/dpi_bridge/plugins/apb/`)**
   - APB-specific DPI-C functions
   - Loads Python driver from `tests/`
   - Exports `dpi_get_transaction()` and `dpi_send_read_data()`

3. **Python Test Infrastructure (`sim/tests/`)**
   - `apb_base.py`: Base classes (`APBTransaction`, `APBSequence`)
   - `apb_driver.py`: DPI interface, loads test modules
   - `apb_*_test.py`: Test-specific stimulus

4. **SystemVerilog Sequence (`apb_requester/apb_python_seq.sv`)**
   - Imports DPI-C functions
   - Fetches transactions from Python via C bridge
   - Drives APB bus with Python-provided data
   - Returns read data to Python

## Prerequisites

- **Xilinx Vivado** (2025.1 or compatible)
- **Python 3** with development headers (`python3-dev`)
- **GCC** compiler
- **UVM library** (included with Vivado)

## Building the DPI Bridge

The modular DPI bridge compiles all source files together:

```bash
cd sim
gcc -shared -fPIC -o libdpi_bridge.so \
    dpi_bridge.c \
    dpi_bridge/core/dpi_core.c \
    dpi_bridge/core/dpi_registry.c \
    dpi_bridge/plugins/apb/apb_plugin.c \
    $(python3-config --cflags --ldflags) \
    -I/tools/Xilinx/2025.1/Vivado/data/xsim/include \
    -I.

# Create symlink for Xilinx xelab
ln -sf libdpi_bridge.so dpi_bridge.so
```

## Running Simulations

### Using sim.py (Recommended)

The `sim.py` tool (located in `Tools/`) supports DPI libraries via the `--sv_lib` argument:

```bash
cd sim
sim.py --top top \
       --filelist apb_inc_xilinx.f \
       --uvm \
       --test apb_init_test \
       --sv_lib dpi_bridge
```

**Arguments:**
- `--top top`: Top-level module name
- `--filelist apb_inc_xilinx.f`: File list for compilation
- `--uvm`: Enable UVM library
- `--test apb_init_test`: UVM test name
- `--sv_lib dpi_bridge`: DPI shared library (without lib prefix or .so extension)

### Selecting Python Tests

Use the `APB_TEST` environment variable to select which Python test to run:

```bash
# Run basic test (default)
APB_TEST=apb_basic_test sim.py --top top --filelist apb_inc_xilinx.f --uvm --test apb_init_test --sv_lib dpi_bridge

# Run burst test
APB_TEST=apb_burst_test sim.py --top top --filelist apb_inc_xilinx.f --uvm --test apb_init_test --sv_lib dpi_bridge

# Run random test
APB_TEST=apb_random_test sim.py --top top --filelist apb_inc_xilinx.f --uvm --test apb_init_test --sv_lib dpi_bridge
```

**Available Tests:**
- `apb_basic_test`: 6 transactions (2 writes, 2 reads, 1 write, 1 read)
- `apb_burst_test`: 16 transactions (8 consecutive writes + 8 reads)
- `apb_random_test`: 20 random read/write transactions

### Cleaning Build Artifacts

```bash
cd sim
sim.py --clean
```

## Python Transaction Driver

The Python driver uses enums and a fluent API for better readability and type safety.

### Using the Fluent API

Edit `sim/apb_driver.py` to customize transactions:

```python
from enum import IntEnum

class APBTransaction(IntEnum):
    """APB transaction types matching SystemVerilog apb_rd_wr_e enum"""
    READ = 0
    WRITE = 1

# Create driver instance
driver = APBDriver()

# Define test sequence using fluent API
driver.write(0x10, 0x12345678) \
      .write(0x14, 0xDEADBEEF) \
      .read(0x10) \
      .read(0x14) \
      .write(0x20, 0xCAFEBABE) \
      .read(0x20)
```

### API Methods

**`write(addr, data)`** - Add APB write transaction
- `addr`: 32-bit address (hex or int)
- `data`: 32-bit data to write
- Returns: `self` (for chaining)

**`read(addr)`** - Add APB read transaction
- `addr`: 32-bit address (hex or int)
- Returns: `self` (for chaining)

### Alternative: Direct Transaction List

You can also define transactions directly:

```python
driver.transactions = [
    (APBTransaction.WRITE, 0x10, 0x12345678),
    (APBTransaction.READ, 0x10, 0),
]
```

## Expected Output

Successful simulation shows Python transactions being executed:

```
[Python] Sending Transaction: Write Addr=0x10 Data=0x12345678
[Python] Sending Transaction: Write Addr=0x14 Data=0xDEADBEEF
[Python] Sending Transaction: Read Addr=0x10 Data=0x0
[Python] Received Read Data: 0x0
[Python] Sending Transaction: Read Addr=0x14 Data=0x0
[Python] Received Read Data: 0x0
[C-Bridge] Python finalized.

--- UVM Report Summary ---
UVM_INFO    : 17
UVM_WARNING : 0
UVM_ERROR   : 0
UVM_FATAL   : 0
```

## DPI-C Integration Details

### SystemVerilog Side

Import DPI-C functions in your sequence:

```systemverilog
import "DPI-C" context function int dpi_init_python();
import "DPI-C" context function int dpi_get_transaction(
    output int is_write, output int addr, output int data);
import "DPI-C" context function void dpi_send_read_data(input int data);
```

### C Bridge Side

The C bridge uses Python's C API:

```c
#include <Python.h>
#include "svdpi.h"

int dpi_init_python() {
    Py_Initialize();
    pModule = PyImport_ImportModule("apb_driver");
    // ...
}
```

### Python Side

Python functions are called from C:

```python
def get_transaction():
    # Return (is_write, addr, data) or None
    
def send_read_data(data):
    # Process read data from simulation
```

## Troubleshooting

### "Invalid path for DPI library"
- Ensure `libdpi_bridge.so` exists in `sim/` directory
- Verify symlink: `dpi_bridge.so -> libdpi_bridge.so`
- Check library was compiled successfully

### "svdpi.h: No such file or directory"
- Verify Xilinx Vivado installation path
- Update include path in gcc command: `-I/path/to/Vivado/data/xsim/include`

### Python import errors during simulation
- Ensure `apb_driver.py` is in `sim/` directory
- Check Python path is set correctly in `dpi_bridge.c`

### UVM errors
- Verify all UVM components are properly connected
- Check test name matches registered test: `apb_init_test`

## Key Features

✅ **Python-driven transactions**: Control APB bus from Python scripts  
✅ **Bidirectional communication**: Read data flows back to Python  
✅ **DPI-C integration**: Efficient C bridge between Python and SV  
✅ **UVM compliant**: Standard UVM testbench architecture  
✅ **Xilinx Vivado support**: Optimized for Xilinx simulation tools  

## License

See [LICENSE](LICENSE) file for details.
