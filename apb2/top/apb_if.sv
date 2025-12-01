interface apb_if(input PCLK);
  logic PRESETn;

  logic [31:0] PADDR;
  logic [31:0] PWDATA;
  logic [31:0] PRDATA;
  logic [2:0]  PPROT;
  logic [3:0]  PSTRB;
  logic        PWRITE;
  logic        PENABLE;
  logic        PREADY;
  logic        PSLVERR;

  // Modports for direct signal access
  modport MDRV_MP(
    input  PCLK, PRESETn, PREADY, PRDATA, PSLVERR,
    output PADDR, PWDATA, PSTRB, PPROT, PWRITE, PENABLE
  );

  modport MMON_MP(
    input PCLK, PRESETn, PADDR, PWDATA, PSTRB, PPROT, PWRITE, PENABLE, PREADY, PRDATA, PSLVERR
  );

  modport SDRV_MP(
    input  PCLK, PRESETn, PADDR, PWDATA, PSTRB, PPROT, PWRITE, PENABLE,
    output PREADY, PRDATA, PSLVERR
  );

  modport SMON_MP(
    input PCLK, PRESETn, PADDR, PWDATA, PSTRB, PPROT, PWRITE, PENABLE, PREADY, PRDATA, PSLVERR
  );

endinterface: apb_if

