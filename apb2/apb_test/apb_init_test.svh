// APB Initial Test to check environment
// Integrity and Basic Bus transaction checks

class apb_init_test extends apb_base_test;
  `uvm_component_utils(apb_init_test)

  reset_seq     reset_seq_h;
  apb_python_seq apb_python_seq_h;

  function new(string name, uvm_component parent);
    super.new(name, parent);
  endfunction

  function void build_phase(uvm_phase phase);
    super.build_phase(phase);
  endfunction

  task run_phase(uvm_phase phase);
    reset_seq_h     = reset_seq::type_id::create("reset_seq_h", this);

    phase.raise_objection(this);

    apb_python_seq_h = apb_python_seq::type_id::create("apb_python_seq_h");

    repeat(1) begin
      reset_seq_h.start(reset_seqr_h);
      apb_python_seq_h.start(m_requester_seqr_h);
    end
    #100;

    phase.drop_objection(this);
  endtask

endclass: apb_init_test

