"""
APB Base Classes and Utilities

Provides reusable infrastructure for APB testing without test-specific stimulus.
"""

from enum import IntEnum

class APBTransactionType(IntEnum):
    """APB transaction types matching SystemVerilog apb_rd_wr_e enum"""
    READ = 0
    WRITE = 1


class APBTransaction:
    """Single APB transaction"""
    
    def __init__(self, addr, data=0, is_write=True):
        """
        Create an APB transaction
        
        Args:
            addr: 32-bit address
            data: 32-bit data (for writes)
            is_write: True for write, False for read
        """
        self.addr = addr
        self.data = data
        self.is_write = is_write
        self.txn_type = APBTransactionType.WRITE if is_write else APBTransactionType.READ
    
    def __repr__(self):
        txn_str = "Write" if self.is_write else "Read"
        return f"APBTransaction({txn_str}, addr=0x{self.addr:X}, data=0x{self.data:X})"


class APBSequence:
    """Base class for APB test sequences"""
    
    def __init__(self, name="APB_Sequence"):
        """
        Create a new APB sequence
        
        Args:
            name: Sequence name for logging
        """
        self.name = name
        self.transactions = []
        self.current_idx = 0
        self.read_data_callbacks = []
    
    def add_write(self, addr, data):
        """
        Add a write transaction
        
        Args:
            addr: 32-bit address
            data: 32-bit data to write
            
        Returns:
            self (for method chaining)
        """
        self.transactions.append(APBTransaction(addr, data, is_write=True))
        return self
    
    def add_read(self, addr, callback=None):
        """
        Add a read transaction
        
        Args:
            addr: 32-bit address
            callback: Optional function to call with read data
            
        Returns:
            self (for method chaining)
        """
        self.transactions.append(APBTransaction(addr, 0, is_write=False))
        self.read_data_callbacks.append(callback)
        return self
    
    def get_next(self, sim_time):
        """
        Get next transaction for DPI bridge
        
        Args:
            sim_time: Current simulation time
            
        Returns:
            Tuple of (is_write, addr, data) or None if no more transactions
        """
        if self.current_idx < len(self.transactions):
            txn = self.transactions[self.current_idx]
            self.current_idx += 1
            
            txn_type_str = "Write" if txn.is_write else "Read"
            print(f"[@{sim_time:>6}] [Python] Sending Transaction: {txn_type_str} Addr=0x{txn.addr:X} Data=0x{txn.data:X}", flush=True)
            
            return (int(txn.txn_type), txn.addr, txn.data)
        else:
            return None
    
    def send_read_data(self, sim_time, data):
        """
        Receive read data from DPI bridge
        
        Args:
            sim_time: Current simulation time
            data: 32-bit read data from APB bus
        """
        print(f"[@{sim_time:>6}] [Python] Received Read Data: 0x{data:X}", flush=True)
        
        # Call registered callback if any
        read_idx = self.current_idx - 1  # Last transaction was the read
        if 0 <= read_idx < len(self.read_data_callbacks):
            callback = self.read_data_callbacks[read_idx]
            if callback is not None:
                callback(data)
    
    def reset(self):
        """Reset sequence to beginning"""
        self.current_idx = 0


class APBRandomSequence(APBSequence):
    """Random APB sequence generator"""
    
    def __init__(self, num_transactions=10, addr_range=(0x0, 0xFF), name="Random_Sequence"):
        """
        Create a random APB sequence
        
        Args:
            num_transactions: Number of random transactions to generate
            addr_range: Tuple of (min_addr, max_addr)
            name: Sequence name
        """
        super().__init__(name)
        import random
        
        for _ in range(num_transactions):
            addr = random.randint(addr_range[0], addr_range[1]) & 0xFFFFFFFC  # Align to 4 bytes
            data = random.randint(0, 0xFFFFFFFF)
            is_write = random.choice([True, False])
            
            if is_write:
                self.add_write(addr, data)
            else:
                self.add_read(addr)
