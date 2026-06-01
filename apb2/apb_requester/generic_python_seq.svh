// Generic Python Sequence - Base Class
//
// Protocol-agnostic sequence that fetches transactions from Python generators.
// Derived classes implement protocol-specific parsing.

class generic_python_seq #(type T = uvm_sequence_item) extends uvm_sequence#(T);
    `uvm_object_param_utils(generic_python_seq#(T))
    
    // Protocol identifier
    string protocol_name;
    
    function new(string name = "generic_python_seq");
        super.new(name);
    endfunction
    
    // Virtual method to parse JSON and create transaction
    // Must be overridden by derived classes
    virtual function T parse_transaction(string json_str);
        `uvm_fatal(get_type_name(), "parse_transaction() must be overridden!")
        return null;
    endfunction
    
    // Virtual method to create response JSON
    // Override if protocol needs to send responses (e.g., read data)
    virtual function string create_response(T rsp);
        return "{}";
    endfunction
    
    task body();
        string txn_json;
        T req_txn;
        int txn_count = 0;
        
        `uvm_info(get_type_name(), 
                 $sformatf("Starting transaction generation for protocol '%s'", protocol_name),
                 UVM_MEDIUM)
        
        forever begin
            // Get next transaction from Python
            txn_json = dpi_get_next_transaction(protocol_name);
            `uvm_info(get_type_name(), $sformatf("Received JSON: %s", txn_json), UVM_LOW)
            
            // Empty string means no more transactions
            if (txn_json == "") begin
                `uvm_info(get_type_name(), 
                         $sformatf("Completed %0d transactions for protocol '%s'", 
                                  txn_count, protocol_name), 
                         UVM_MEDIUM)
                break;
            end
            
            // Parse JSON into transaction object
            `uvm_info(get_type_name(), "Parsing transaction...", UVM_LOW)
            req_txn = parse_transaction(txn_json);
            `uvm_info(get_type_name(), "Parsed transaction", UVM_LOW)
            
            if (req_txn == null) begin
                `uvm_error(get_type_name(), 
                          $sformatf("Failed to parse transaction: %s", txn_json))
                continue;
            end
            
            // Send to driver
            `uvm_info(get_type_name(), "Starting item...", UVM_LOW)
            start_item(req_txn);
            finish_item(req_txn);
            txn_count++;
            
            // Send response back to Python (for reads, etc.)
            dpi_send_response(protocol_name, create_response(req_txn));
        end
    endtask
    
    // Helper function to find substring in string
    function int str_find(string str, string sub_str, int start_pos = 0);
        int str_len = str.len();
        int sub_len = sub_str.len();
        
        if (sub_len == 0) return start_pos;
        if (str_len < sub_len) return -1;
        
        for (int i = start_pos; i <= str_len - sub_len; i++) begin
            if (str.substr(i, i + sub_len - 1) == sub_str) begin
                return i;
            end
        end
        return -1;
    endfunction

    // Helper function to extract string field from JSON
    function string extract_string_field(string json, string field_name);
        int start_pos, end_pos;
        string search_str = $sformatf("\"%s\":", field_name);
        
        start_pos = str_find(json, search_str);
        if (start_pos < 0) return "";
        
        start_pos += search_str.len();
        
        // Skip whitespace
        while (start_pos < json.len() && (json[start_pos] == " " || json[start_pos] == "\t"))
            start_pos++;
            
        // Check for opening quote
        if (json[start_pos] != "\"") return "";
        start_pos++; // Skip quote
        
        end_pos = str_find(json, "\"", start_pos);
        
        if (end_pos > start_pos) begin
            return json.substr(start_pos, end_pos-1);
        end
        
        return "";
    endfunction
    
    // Helper function to extract integer from JSON
    function int extract_int_field(string json, string field_name);
        int start_pos, end_pos;
        string search_str = $sformatf("\"%s\":", field_name);
        string value_str;
        
        start_pos = str_find(json, search_str);
        if (start_pos < 0) return 0;
        
        start_pos += search_str.len();
        
        // Skip whitespace
        while (start_pos < json.len() && (json[start_pos] == " " || json[start_pos] == "\t"))
            start_pos++;
        
        // Find next comma, closing brace, or quote
        end_pos = start_pos;
        while (end_pos < json.len() && 
               json[end_pos] != "," && 
               json[end_pos] != "}" && 
               json[end_pos] != "\"")
            end_pos++;
        
        if (end_pos > start_pos) begin
            value_str = json.substr(start_pos, end_pos-1);
            return value_str.atoi();
        end
        
        return 0;
    endfunction
endclass
