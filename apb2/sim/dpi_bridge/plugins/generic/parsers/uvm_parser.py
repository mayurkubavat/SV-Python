import re

class UVMObjectParser:
    """
    Base class for parsing UVM line printer output.
    
    UVM line printer format: "field1: value1 field2: value2 ..."
    """
    
    @staticmethod
    def parse_hex(value_str):
        """Convert UVM hex format ('hXXXX) to Python int."""
        if value_str.startswith("'h"):
            return int(value_str[2:], 16)
        elif value_str.startswith("0x"):
            return int(value_str, 16)
        return int(value_str)
    
    @staticmethod
    def parse_field(field_name, uvm_string):
        """Extract a single field value from UVM string."""
        # Pattern: field_name: value (stops at next space or end of string)
        pattern = rf"{field_name}:\s*(\S+)"
        match = re.search(pattern, uvm_string)
        if match:
            return match.group(1)
        return None
    
    def parse(self, uvm_string):
        """
        Parse UVM string into dictionary.
        Override this in subclasses to define field mappings.
        """
        raise NotImplementedError("Subclasses must implement parse()")



