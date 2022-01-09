`timescale 1us/1us


`define CNTRL_PREG_A  3'b001
`define CNTRL_PREG_B  3'b101
`define CNTRL_DDR_A   3'b000
`define CNTRL_DDR_B   3'b100
`define CNTRL_CRA     3'b010
`define CNTRL_CRA_ALT 3'b011
`define CNTRL_CRB     3'b110
`define CNTRL_CRB_ALT 3'b111

`define CAINTWORD {CRA[1:0], CA1}
`define CBINTWORD {CRB[1:0], CB1}
`define CRGWORD {RS[1:0], CRA[2]} // WE JUST NEED A to Infer B


`define CA2ASOUTWORD {CRA[4], CRA[3]}
`define CB2ASOUTWORD {CRB[4], CRB[3]}

module MC6820B(
        input [ 7:0] DI,		    // DATA INPUT
        output reg [ 7:0] DO,   // DATA OUT

        input [ 7:0] PAI,		    // PERIFERIAL A INPUT
        output reg [ 7:0] PAO,  // PERIFERIAL A OUT

        input [ 7:0] PBI,       // PERIFERIAL B INPUT
        output reg [ 7:0] PBO,  // PERIFERIAL B OUT

        input CA1,		          // INTERRUPT INPUT A
        input CB1,		          // INTERRUPT INPUT B

        input CA2I,		          // PRIPHERIAL CONTROL A INPUT
        output reg CA2O,        // PRIPHERIAL CONTROL A OUT
        input CB2I,		          // PRIPHERIAL CONTROL B INPUT
        output reg CB2O,        // PRIPHERIAL CONTROL B OUT

        input [ 2:0] CS,		    // CHIP SELECT (0=1, 1=1, 2=0 to CHIPSELECT)
        input [ 1:0] RS,		    // REG SELECT

        input rw,				        // DATA (0=READ, 1=WRITE)
        input enable,               // CLK
        input nreset,			    // RESET (1=OFF, 0=ON)
        output wire irqA,            // IRQ A
        output reg irqB             // IRQ B
    );


    // INTERNAL REGISTERS
    reg [7:0] CRA = 8'b00000000;        // CONTROL REG A
    reg [7:0] CRB = 8'b00000000;        // CONTROL REG B
    reg [7:0] DDRA = 8'b00000000;       // DATA DIRECTION REGISTER A (0=IN, 1=OUT)
    reg [7:0] DDRB = 8'b00000000;       // DATA DIRECTION REGISTER B (0=IN, 1=OUT)
    reg [7:0] DIR = 8'b00000000;        // DATA INPUT REGISTER
    wire isDeselected;                       // Convenient selected status reg
    wire PeriferialASelected;


    // ISCA
    reg ISCA1 = 0;                      // Interrupt Status Control logic A SC1
    reg ISCA2 = 0;                      // Interrupt Status Control logic A SC2

    // ORA
    reg ORA;                            // Peripheral Output Registers

    // CRA/B
    // -------------------------------------------------------
    //     7   |   6    | 5 | 4 | 3 |   2    |    1   |   0
    // -------------------------------------------------------
    //   IRQ1  |  IRQ2  |    CA2    | DDRA/B |    CA1 Control
    // -------------------------------------------------------

    assign isDeselected = (CS[0] & CS[1]) ^ CS[2];

    /*

    // CA2O
    always @(*) begin: CA2O_BLOCK
        if (~nreset) begin
            CA2O = 0;
        end else begin
            CA2O = 0;
            if (CRA[5]) begin
                if (CRA[4:3] == 2'b00) begin
                    if (!enable && RS[1:0] == 2'b00 && CRA[2] == 1 && rw) begin
                        // LOW on Negative transition of E after an MPU READ on A Data
                        CA2O = 0;
                    end else if (CRA[7]) begin
                        // High when the interrupt flag bit CRB-7 is set by an active transition of CB1
                        CA2O = 1;
                    end
                end else if (CRA[4:3] == 2'b01) begin
                    if (!enable) begin
                        if (!isDeselected && !enable) begin
                            // High on the negative edge of the first E pulse wich occur durinng a deselect
                            CA2O = 1;
                        end else if (RS[1:0] == 2'b00 && CRA[2] == 1 && rw) begin
                            // LOW on Negative transition of E after an MPU READ on A Data
                            CA2O = 0;
                        end
                        
                    end
                end else if (CRA[4:3] == 2'b10) begin
                    if (!CRA[3]) begin
                        // Always low as long as CRA-3 is low.
                        CA2O = 0;
                    end else if (~rw && RS[1:0] == 2'b01) begin
                        // TODO: ARE WE SURE IT IS HIGH?
                        // Will go high on an MPU write to Control Register A that changes CRA-3 to 'one'
                        CA2O = 1;
                    end
                end else if (CRA[4:3] == 2'b11) begin
                    if (CRA[3]) begin
                        // Always high when CRA-3 is high
                        CA2O=1;
                    end else if (~rw && RS[1:0] == 2'b01) begin
                        // Low as a resgister CRA-3 is cleared by a write ops
                        CA2O=0;
                    end 
                end
            end
        end
    end

    */


    assign PeriferialASelected = RS[1:0] == 2'b00 && CRA[2] == 1;
    wire aIsLocked;
    IRQControl IRQAControl(
                   .clk(enable),
                   .readp(PeriferialASelected && rw),
                   .deselect(!isDeselected),
                   .nreset(nreset),
                   .isLocked(aIsLocked)
               );

    // IRQA
    assign irqA = !((CRA[0] && ISCA1) || (CRA[3] && ISCA2));

    always @(*) begin
        if (PeriferialASelected && rw) begin
            ISCA1 = 0;
        end
        else begin
            ISCA1 = (!(CA1 ^ CRA[1]) && !aIsLocked) || CRA[7];
        end
    end

    always @(*) begin
        if (PeriferialASelected && rw) begin
            ISCA2 = 0;
        end
        else begin
            ISCA2 = (!(CA2I ^ CRA[4]) && !aIsLocked) || CRA[6];
        end
    end

    // CRA[7] CA1 AS INTERRUPT INPUT
    always @(posedge enable, negedge nreset) begin: CRA7_BLOCK
        if (~nreset) begin
            CRA[7] <= 0;
        end
        else begin
            CRA[7] <= ISCA1;
        end
    end

    // CRA[6] CA2 AS INTERRUPT INPUT
    always @(posedge enable, negedge nreset) begin: CRA6_BLOCK
        if (~nreset) begin
            CRA[6] <= 0;
        end
        else begin
            CRA[6] <= ISCA2;
        end
    end

    // DO
    always @(posedge enable) begin: DO_BLOCK
        if (!isDeselected) begin
            DO <= 0; // High Pendence
        end else begin
            if (!rw) begin
                DO <= 0; // High Pendence
            end
            else begin
                if (RS[1:0] == 2'b01) begin
                    DO <= CRA;
                end
                else if (PeriferialASelected) begin
                    DO <= PAI;
                end
                else if (RS[1:0] == 2'b00 && CRA[2] == 0) begin
                    DO <= DDRA;
                end
            end
        end
    end

    // DIR - DATA INPUT REGISTER
    always @(negedge enable) begin: DIR_BLOCK
        if (~nreset) begin
            DIR <= 0;
        end
        else begin
            if (~rw) begin
                DIR <= DI;
            end
        end
    end

    // CRA[5:0]
    always @(posedge enable, negedge nreset) begin: CRA50_BLOCK
        if (~nreset) begin
            CRA[5:0] <= 0;
        end
        else begin
            if (~rw && RS[1:0] == 2'b01) begin
                CRA[5:0] <= DIR[5:0];
            end
        end
    end

    // PAO
    always @(posedge enable) begin: PAO_BLOCK
        if (nreset) begin
            if (~rw && PeriferialASelected) begin
                PAO <= DIR;
            end
        end
    end

    // DDRA
    always @(posedge enable, negedge nreset) begin: DDRA_BLOCK
        if (~nreset) begin
            DDRA <= 0;
        end
        else begin
            if (~rw && RS[1:0] == 2'b00 && CRA[2] == 0) begin
                DDRA <= DIR;
            end
        end
    end


endmodule
