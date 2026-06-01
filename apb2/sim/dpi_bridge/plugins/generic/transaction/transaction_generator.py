"""
Generic Transaction Generator Framework

Provides protocol-agnostic transaction generation using factory pattern.
Python generators create transactions as JSON, SV parses and executes.
"""

import json
import sys
from abc import ABC, abstractmethod
from typing import Optional, Dict, Any

class TransactionGenerator(ABC):
    """Base class for protocol-specific transaction generators"""
    
    @abstractmethod
    def get_next(self, sim_time: int) -> Optional[Dict[str, Any]]:
        """
        Generate next transaction.
        
        Args:
            sim_time: Current simulation time
            
        Returns:
            Dictionary with transaction fields, or None if no more transactions
        """
        pass
    
    @abstractmethod
    def send_response(self, sim_time: int, response_data: Dict[str, Any]):
        """
        Handle response from DUT (e.g., read data).
        
        Args:
            sim_time: Current simulation time
            response_data: Response from DUT
        """
        pass


class APBGenerator(TransactionGenerator):
    """APB protocol transaction generator"""
    
    def __init__(self):
        self.transaction_queue = []
        self.current_idx = 0
        
    def add_write(self, addr: int, data: int, strobe: int = 0xF):
        """Add write transaction to queue"""
        self.transaction_queue.append({
            "type": "write",
            "addr": addr,
            "data": data,
            "strobe": strobe
        })
    
    def add_read(self, addr: int):
        """Add read transaction to queue"""
        self.transaction_queue.append({
            "type": "read",
            "addr": addr,
            "data": 0,
            "strobe": 0xF
        })
    
    def get_next(self, sim_time: int) -> Optional[Dict[str, Any]]:
        """Get next APB transaction"""
        if self.current_idx >= len(self.transaction_queue):
            return None  # No more transactions
        
        txn = self.transaction_queue[self.current_idx]
        self.current_idx += 1
        
        print(f"[@{sim_time}] [Python] Generating APB {txn['type']}: "
              f"addr=0x{txn['addr']:X} data=0x{txn['data']:X}", flush=True)
        
        return {
            "protocol": "apb",
            "fields": txn
        }
    
    def send_response(self, sim_time: int, response_data: Dict[str, Any]):
        """Handle APB read response"""
        if "data" in response_data:
            print(f"[@{sim_time}] [Python] APB Read Response: "
                  f"addr=0x{response_data.get('addr', 0):X} "
                  f"data=0x{response_data['data']:X}", flush=True)


# Global registry of generators
_generators: Dict[str, TransactionGenerator] = {}


def register_generator(name: str, generator: TransactionGenerator):
    """Register a transaction generator for a protocol"""
    _generators[name] = generator
    print(f"[Python] Registered generator: {name}", flush=True)


def dpi_get_next_transaction(protocol_name: str) -> str:
    """
    DPI-C callable: Get next transaction for protocol.
    
    Args:
        protocol_name: Protocol identifier (e.g., "apb", "axi")
        
    Returns:
        JSON string with transaction data, or empty string if done
    """
    if protocol_name not in _generators:
        print(f"[Python] ERROR: Unknown protocol '{protocol_name}'", flush=True)
        return ""
    
    gen = _generators[protocol_name]
    txn = gen.get_next(0)  # sim_time can be passed if needed
    
    if txn is None:
        print(f"[Python] No more transactions for '{protocol_name}'", flush=True)
        return ""  # Empty string signals end of transactions
    
    return json.dumps(txn)


def dpi_send_response(protocol_name: str, response_json: str):
    """
    DPI-C callable: Send response back to generator.
    
    Args:
        protocol_name: Protocol identifier
        response_json: JSON string with response data
    """
    if protocol_name not in _generators:
        return
    
    gen = _generators[protocol_name]
    
    if response_json:
        try:
            response = json.loads(response_json)
            gen.send_response(0, response)
        except json.JSONDecodeError:
            print(f"[Python] ERROR: Invalid JSON in response: {response_json}", flush=True)


def dpi_setup_apb_test():
    """
    DPI-C callable: Setup APB test.
    """
    setup_apb_test()
    print("[Python] APB test setup via DPI complete", flush=True)


def setup_apb_test():
    """Example: Setup APB test with predefined transactions"""
    apb_gen = APBGenerator()
    
    # Add some test transactions
    apb_gen.add_write(0x1000, 0xDEADBEEF)
    apb_gen.add_write(0x2000, 0xCAFEBABE)
    apb_gen.add_read(0x1000)
    apb_gen.add_read(0x2000)
    apb_gen.add_write(0x3000, 0x12345678, 0x3)  # Half-word write
    
    register_generator("apb", apb_gen)
    print("[Python] APB test setup complete with 5 transactions", flush=True)
