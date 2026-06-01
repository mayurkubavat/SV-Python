// APB Python Generator Sequence
//
// APB-specific implementation of generic_python_seq.
// Parses JSON transactions from Python and creates APB transaction objects.

class apb_python_gen_seq extends generic_python_seq#(apb_xtn);
    `uvm_object_utils(apb_python_gen_seq)
    
    function new(string name = "apb_python_gen_seq");
        super.new(name);
        protocol_name = "apb";  // Set protocol identifier
    endfunction
    
    // Parse JSON into APB transaction
    virtual function apb_xtn parse_transaction(string json_str);
        apb_xtn txn;
        string txn_type;
        int addr, data, strobe;
        
        txn = apb_xtn::type_id::create("txn");
        
        // Expected JSON format:
        // {"protocol":"apb","fields":{"type":"write","addr":4096,"data":3735928559,"strobe":15}}
        
        // Extract transaction type
        txn_type = extract_string_field(json_str, "type");
        if (txn_type == "write") begin
            txn.apb_rd_wr = apb_xtn::APB_WRITE;
        end else if (txn_type == "read") begin
            txn.apb_rd_wr = apb_xtn::APB_READ;
        end else begin
            `uvm_error(get_type_name(), $sformatf("Unknown transaction type: %s", txn_type))
            return null;
        end
        
        // Extract address
        addr = extract_int_field(json_str, "addr");
        txn.apb_address = addr;
        
        // Extract data (for writes)
        data = extract_int_field(json_str, "data");
        txn.apb_wr_data = data;
        
        // Extract strobe
        strobe = extract_int_field(json_str, "strobe");
        txn.apb_strobe = strobe;
        
        `uvm_info(get_type_name(), 
                 $sformatf("Parsed APB %s: addr=0x%0h data=0x%0h strobe=0x%0h", 
                          txn.apb_rd_wr.name(), txn.apb_address, txn.apb_wr_data, txn.apb_strobe),
                 UVM_HIGH)
        
        return txn;
    endfunction
    
    // Create response for Python (send read data back)
    virtual function string create_response(apb_xtn rsp);
        string response;
        
        if (rsp.apb_rd_wr == apb_xtn::APB_READ) begin
            // Send read data back to Python
            response = $sformatf("{\"addr\":%0d,\"data\":%0d}", 
                                rsp.apb_address, rsp.apb_rd_data);
        end else begin
            // No response needed for writes
            response = "{}";
        end
        
        return response;
    endfunction
endclass
