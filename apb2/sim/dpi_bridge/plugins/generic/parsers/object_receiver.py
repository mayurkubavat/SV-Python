import sys
from apb_parser import APBTransactionParser

def receive_object(tag, object_str):
    """
    Receives a UVM object string from SystemVerilog, parses it, and prints the object.
    
    Args:
        tag (str): Identifier for the object type (e.g., "apb_xtn_uvm")
        object_str (str): The serialized string from UVM sprint()
    """
    print(f"[Python] Received Object. Tag: {tag}")
    sys.stdout.flush()
    
    # Dispatch based on tag
    if tag == "apb_xtn_uvm":
        print(f"[Python] Parsing APB Transaction...")
        parser = APBTransactionParser()
        data = parser.parse(object_str)
        print(f"[Python] Parsed Data: {data}")
        parser.print_transaction(data)
        sys.stdout.flush()
    
    # Add other protocols here
    # elif tag == "axi_txn":
    #     parser = AXITransactionParser()
    #     ...
        
    else:
        print(f"[Python] Warning: Unknown tag '{tag}'. Raw string: {object_str}")
        sys.stdout.flush()
