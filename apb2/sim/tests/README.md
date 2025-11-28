# Python Test Structure

## Overview

All Python test-related code is now consolidated in the `tests/` directory with clear separation between infrastructure and test stimulus.

## Directory Structure

```
sim/tests/
├── apb_base.py           # Base classes (APBTransaction, APBSequence)
├── apb_driver.py         # DPI interface (loads tests dynamically)
├── apb_basic_test.py     # Basic read/write test
├── apb_burst_test.py     # Burst transactions
└── apb_random_test.py    # Random stimulus
```

## Components

### apb_base.py - Reusable Infrastructure

**APBTransaction**: Single transaction
```python
txn = APBTransaction(addr=0x100, data=0xDEAD, is_write=True)
```

**APBSequence**: Base sequence class
```python
seq = APBSequence("My_Test")
seq.add_write(0x10, 0x1234)
seq.add_read(0x10)
```

**APBRandomSequence**: Random generator
```python
seq = APBRandomSequence(num_transactions=20, addr_range=(0x0, 0xFF))
```

### apb_driver.py - DPI Interface

- Loads test via `load_test(test_name)`
- Provides DPI-C callable functions
- Auto-loads test from `APB_TEST` environment variable

### tests/*.py - Test Stimulus

Each test file must have:
```python
def create_sequence():
    seq = APBSequence("Test_Name")
    # Add transactions
    return seq
```

## Running Tests

### Method 1: Environment Variable
```bash
APB_TEST=apb_burst_test sim.py --top top --filelist apb_inc_xilinx.f --uvm --test apb_init_test --sv_lib dpi_bridge
```

### Method 2: Default (apb_basic_test)
```bash
sim.py --top top --filelist apb_inc_xilinx.f --uvm --test apb_init_test --sv_lib dpi_bridge
```

## Creating New Tests

1. Create `sim/tests/my_test.py`:
```python
import sys, os
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
from apb_base import APBSequence

def create_sequence():
    seq = APBSequence("My_Test")
    seq.add_write(0x100, 0xABCD)
    seq.add_read(0x100)
    return seq
```

2. Run:
```bash
APB_TEST=my_test sim.py ...
```

## Example Tests

### Basic Test
- 2 writes (0x10, 0x14)
- 2 reads (0x10, 0x14)
- 1 write + 1 read (0x20)

### Burst Test
- 8 consecutive writes (0x1000-0x101C)
- 8 consecutive reads (0x1000-0x101C)

### Random Test
- 20 random read/write transactions
- Address range: 0x0-0x1FF
- Aligned to 4-byte boundaries

## Benefits

✅ **Separation of Concerns**: Infrastructure vs. stimulus
✅ **Reusability**: Base classes shared across tests
✅ **Easy Test Creation**: Just implement `create_sequence()`
✅ **Dynamic Loading**: Switch tests via environment variable
✅ **No C/SV Changes**: Add tests without recompilation
