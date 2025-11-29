from uvm_parser import UVMObjectParser

class APBTransactionParser(UVMObjectParser):
    """Parser for apb_xtn UVM objects."""
    
    def parse(self, uvm_string):
        """Parse APB transaction from UVM line printer output."""
        data = {}
        
        # Extract hex fields
        hex_fields = ['apb_address', 'apb_wr_data', 'apb_rd_data']
        for field in hex_fields:
            value_str = self.parse_field(field, uvm_string)
            if value_str:
                data[field] = self.parse_hex(value_str)
        
        # Extract integer fields
        int_fields = ['apb_enable', 'apb_strobe', 'apb_ready', 
                      'apb_completer_err', 'apb_prot', 'apb_en_delay']
        for field in int_fields:
            value_str = self.parse_field(field, uvm_string)
            if value_str:
                try:
                    data[field] = int(value_str)
                except ValueError:
                    data[field] = value_str
        
        # Extract enum field (apb_rd_wr)
        rd_wr = self.parse_field('apb_rd_wr', uvm_string)
        if rd_wr:
            data['apb_rd_wr'] = rd_wr
        
        return data
    
    def print_transaction(self, data):
        """Pretty print APB transaction."""
        print(f"  [APB Transaction]")
        if 'apb_address' in data:
            print(f"  - Address: {hex(data['apb_address'])}")
        if 'apb_rd_wr' in data:
            print(f"  - Type: {data['apb_rd_wr']}")
        if 'apb_wr_data' in data:
            print(f"  - Write Data: {hex(data['apb_wr_data'])}")
        if 'apb_rd_data' in data:
            print(f"  - Read Data: {hex(data['apb_rd_data'])}")
        if 'apb_prot' in data:
            print(f"  - Protection: {data['apb_prot']}")
