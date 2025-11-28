"""
APB Driver - DPI Interface

Provides the DPI-C callable interface for SystemVerilog.
Test stimulus is loaded from separate test files.
"""

import sys
import os

# Add tests directory to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'tests'))

from apb_base import APBSequence

# Global sequence - set by test
current_sequence = None

def load_test(test_name):
    """
    Load test sequence from test module
    
    Args:
        test_name: Name of test module (e.g., 'apb_basic_test')
    """
    global current_sequence
    
    try:
        # Import test module
        test_module = __import__(test_name)
        
        # Get sequence from test
        if hasattr(test_module, 'create_sequence'):
            current_sequence = test_module.create_sequence()
            print(f"[Python] Loaded test: {test_name}", flush=True)
        else:
            print(f"[Python] Error: Test '{test_name}' missing create_sequence()", flush=True)
            current_sequence = None
    except ImportError as e:
        print(f"[Python] Error loading test '{test_name}': {e}", flush=True)
        current_sequence = None

# DPI-C callable functions
def get_transaction(sim_time):
    """
    Called from C bridge to get next transaction
    
    Args:
        sim_time: Current simulation time
        
    Returns:
        Tuple of (is_write, addr, data) or None
    """
    if current_sequence is None:
        return None
    
    return current_sequence.get_next(sim_time)

def send_read_data(sim_time, data):
    """
    Called from C bridge to send read data
    
    Args:
        sim_time: Current simulation time
        data: 32-bit read data
    """
    if current_sequence is not None:
        current_sequence.send_read_data(sim_time, data)

# Auto-load default test if not loaded via DPI
if current_sequence is None:
    # Check for TEST_NAME environment variable or plusarg
    test_name = os.environ.get('APB_TEST', 'apb_basic_test')
    load_test(test_name)
