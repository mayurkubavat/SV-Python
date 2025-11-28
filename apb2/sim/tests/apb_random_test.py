"""
APB Random Test

Generates random APB transactions for stress testing.
"""

from apb_base import APBRandomSequence

def create_sequence():
    """
    Create random test sequence
    
    Returns:
        APBRandomSequence with random stimulus
    """
    # Generate 20 random transactions in address range 0x0-0x1FF
    seq = APBRandomSequence(
        num_transactions=20,
        addr_range=(0x0, 0x1FF),
        name="Random_Test"
    )
    
    return seq
