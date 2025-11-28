"""
APB Basic Test

Simple test with basic read/write transactions.
"""

from apb_base import APBSequence

def create_sequence():
    """
    Create the test sequence
    
    Returns:
        APBSequence configured with test stimulus
    """
    seq = APBSequence("Basic_Test")
    
    # Write transactions
    seq.add_write(0x10, 0x12345678)
    seq.add_write(0x14, 0xDEADBEEF)
    
    # Read transactions
    seq.add_read(0x10)
    seq.add_read(0x14)
    
    # More writes and reads
    seq.add_write(0x20, 0xCAFEBABE)
    seq.add_read(0x20)
    
    return seq
