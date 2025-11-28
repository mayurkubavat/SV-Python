class apb_env extends uvm_env;
  `uvm_component_utils(apb_env)

  apb_env_config m_env_cfg;

  // Configuration for ABP Requester and Completer
  apb_requester_config m_requester_cfg;
  apb_completer_config m_completer_cfg;

  // APB Subscriber to generate coverage
  apb_subscriber apb_subscriber_h;

  apb_requester apb_requester_h;
  apb_completer apb_completer_h;
  reset_agent   reset_agent_h;

  apb_seqr   m_requester_seqr_h;
  apb_seqr   m_completer_seqr_h;
  reset_seqr reset_seqr_h;

  function new(string name, uvm_component parent);
    super.new(name, parent);
  endfunction

  function void build_phase(uvm_phase phase);
    if(!uvm_config_db#(apb_env_config)::get(this, "", "apb_env_config", m_env_cfg))
      `uvm_fatal("APB_ENV", {get_full_name(), " Cannot get environmet configuration object from test"})

    if(m_env_cfg.has_coverage) begin
      apb_subscriber_h = apb_subscriber::type_id::create("apb_subscriber_h", this);
    end

    // Set master agent(APB Bridge) configuration
    m_requester_cfg           = apb_requester_config::type_id::create("m_requester_cfg");
    m_requester_cfg.apb_intf  = m_env_cfg.apb_intf;
    m_requester_cfg.is_active = m_env_cfg.requester_is_active;
    uvm_config_db#(apb_requester_config)::set(this, "apb_requester*", "apb_requester_config", m_requester_cfg);

    // Set completer agent(APB Completer) configuration
    m_completer_cfg           = apb_completer_config::type_id::create("m_completer_cfg");
    m_completer_cfg.apb_intf  = m_env_cfg.apb_intf;
    m_completer_cfg.is_active = m_env_cfg.completer_is_active;
    uvm_config_db#(apb_completer_config)::set(this, "apb_completer*", "apb_completer_config", m_completer_cfg);

    apb_requester_h = apb_requester::type_id::create("apb_requester_h", this);
    apb_completer_h = apb_completer::type_id::create("apb_completer_h", this);
    reset_agent_h   = reset_agent::type_id::create("reset_agent_h", this);
  endfunction

  function void connect_phase(uvm_phase phase);
    m_requester_seqr_h = apb_requester_h.m_requester_seqr_h;
    m_completer_seqr_h  = apb_completer_h.m_completer_seqr_h;
    reset_seqr_h    = reset_agent_h.reset_seqr_h;
  endfunction

endclass: apb_env

