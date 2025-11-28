from enum import IntEnum

class APBTransaction(IntEnum):
    """APB transaction types matching SystemVerilog apb_rd_wr_e enum"""
    READ = 0
    WRITE = 1

class APBDriver:
    """Python driver for APB transactions with helper APIs"""
    
    def __init__(self):
        self.transactions = []
        self.current_idx = 0
        
    def write(self, addr, data):
        """Add APB write transaction
        
        Args:
            addr: 32-bit address
            data: 32-bit data to write
        """
        self.transactions.append((APBTransaction.WRITE, addr, data))
        return self
    
    def read(self, addr):
        """Add APB read transaction
        
        Args:
            addr: 32-bit address
        """
        self.transactions.append((APBTransaction.READ, addr, 0))
        return self
    
    def get_transaction(self, sim_time):
        """Get next transaction for DPI bridge
        
        Args:
            sim_time: Current simulation time
            
        Returns:
            Tuple of (is_write, addr, data) or None if no more transactions
        """
        if self.current_idx < len(self.transactions):
            txn = self.transactions[self.current_idx]
            self.current_idx += 1
            txn_type = "Write" if txn[0] == APBTransaction.WRITE else "Read"
            print(f"[@{sim_time:>6}] [Python] Sending Transaction: {txn_type} Addr=0x{txn[1]:X} Data=0x{txn[2]:X}", flush=True)
            return txn
        else:
            return None
    
    def send_read_data(self, sim_time, data):
        """Receive read data from DPI bridge
        
        Args:
            sim_time: Current simulation time
            data: 32-bit read data from APB bus
        """
        print(f"[@{sim_time:>6}] [Python] Received Read Data: 0x{data:X}", flush=True)


# Global driver instance
driver = APBDriver()

# Define test sequence using fluent API
driver.write(0x10, 0x12345678) \
      .write(0x14, 0xDEADBEEF) \
      .read(0x10) \
      .read(0x14) \
      .write(0x20, 0xCAFEBABE) \
      .read(0x20)

# DPI-C callable functions
def get_transaction(sim_time):
    """Called from C bridge to get next transaction"""
    return driver.get_transaction(sim_time)

def send_read_data(sim_time, data):
    """Called from C bridge to send read data"""
    driver.send_read_data(sim_time, data)
