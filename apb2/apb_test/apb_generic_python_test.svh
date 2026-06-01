    class apb_generic_python_test extends apb_base_test;
        `uvm_component_utils(apb_generic_python_test)
        
        function new(string name, uvm_component parent);
            super.new(name, parent);
        endfunction
        
        task run_phase(uvm_phase phase);
            apb_python_gen_seq seq;
            reset_seq rst_seq;
            
            phase.raise_objection(this);
            
            // Initialize Python
            if (dpi_init_python() != 0) begin
                `uvm_fatal(get_type_name(), "Failed to initialize Python")
            end
            
            // Perform Reset
            rst_seq = reset_seq::type_id::create("rst_seq");
            rst_seq.start(reset_seqr_h);
            
            // Setup test in Python (register generator with transactions)
            dpi_setup_apb_test();
            
            // Create and run sequence
            seq = apb_python_gen_seq::type_id::create("seq");
            seq.start(m_requester_seqr_h);
            
            #100ns; // Allow time for transactions to complete
            
            phase.drop_objection(this);
        endtask
    endclass
