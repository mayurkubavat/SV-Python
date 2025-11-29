    class apb_dpi_object_test extends uvm_test;
        `uvm_component_utils(apb_dpi_object_test)

        function new(string name, uvm_component parent);
            super.new(name, parent);
        endfunction

        task run_phase(uvm_phase phase);
            apb_xtn xtn;
            uvm_line_printer line_printer;
            
            phase.raise_objection(this);
            
            // Initialize DPI bridge
            if (dpi_init_python() != 0) begin
                `uvm_fatal("DPI_OBJECT_TEST", "Failed to initialize DPI bridge")
            end
            
            `uvm_info("DPI_OBJECT_TEST", "Creating and sending APB transactions...", UVM_LOW)

            // Create line printer for serialization
            line_printer = new();
            line_printer.knobs.reference = 0; // Don't print object references

            // Create a Write Transaction
            xtn = apb_xtn::type_id::create("xtn_write");
            xtn.apb_address = 32'h1000_0000;
            xtn.apb_wr_data = 32'hDEAD_BEEF;
            xtn.apb_rd_wr = apb_xtn::APB_WRITE;
            xtn.apb_prot = 3'b001;
            
            `uvm_info("DPI_OBJECT_TEST", $sformatf("Sending Write XTN: %s", xtn.sprint(line_printer)), UVM_LOW)
            dpi_send_object("apb_xtn_uvm", xtn.sprint(line_printer));

            #100ns;

            // Create a Read Transaction
            xtn = apb_xtn::type_id::create("xtn_read");
            xtn.apb_address = 32'h2000_0000;
            xtn.apb_rd_wr = apb_xtn::APB_READ;
            
            `uvm_info("DPI_OBJECT_TEST", $sformatf("Sending Read XTN: %s", xtn.sprint(line_printer)), UVM_LOW)
            dpi_send_object("apb_xtn_uvm", xtn.sprint(line_printer));

            #100ns;
            
            phase.drop_objection(this);
        endtask

    endclass


