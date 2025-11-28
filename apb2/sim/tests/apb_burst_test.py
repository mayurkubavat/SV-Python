"""
APB Burst Test

Test with burst write and read transactions to consecutive addresses.
"""

from apb_base import APBSequence

def create_sequence():
    """
    Create burst test sequence
    
    Returns:
        APBSequence configured with burst stimulus
    """
    seq = APBSequence("Burst_Test")
    
    base_addr = 0x1000
    burst_length = 8
    
    # Burst write
    for i in range(burst_length):
        addr = base_addr + (i * 4)
        data = 0xA0000000 + i
        seq.add_write(addr, data)
    
    # Burst read
    for i in range(burst_length):
        addr = base_addr + (i * 4)
        seq.add_read(addr)
    
    return seq
