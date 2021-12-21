`timescale 1ns/1ps


`define CNTRL_PREG_A  3'b001
`define CNTRL_PREG_B  3'b101
`define CNTRL_DDR_A   3'b000
`define CNTRL_DDR_B   3'b100
`define CNTRL_CRA     3'b010
`define CNTRL_CRA_ALT 3'b011
`define CNTRL_CRB     3'b110
`define CNTRL_CRB_ALT 3'b111

`define CHIP_SELECTED 3'b011

module MC6820(
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
        input enable,           // CLK
        input reset_n,			    // RESET (1=OFF, 0=ON)
        output reg irqA,        // IRQ A
        output reg irqB         // IRQ B
    );

    // INTERNAL REGISTERS
    reg [7:0] CRA;        // CONTROL REG A
    reg [7:0] CRB;        // CONTROL REG B
    reg [7:0] DDRA;       // DATA DIRECTION REGISTER A (0=IN, 1=OUT)
    reg [7:0] DDRB;       // DATA DIRECTION REGISTER B (0=IN, 1=OUT)

    reg [2:0] CTRLWORD;   // CNTRL AGGREGATED WORD

    initial begin
        irqA <= 1;
        irqB <= 1;
    end






    always @(posedge enable or negedge reset_n)
        if (!reset_n)
            reset();
        else begin

            // INTERRUPTS A
            if (!CRA[1] && !CA1) begin
                $display("!CRA[1] && !CA1");
                CRA[7] = 1;
                if (CRA[0]) begin
                    irqA <= 0;
                end
            end
            else if (CRA[1] && CA1) begin
                $display("CRA[1] && CA1");
                CRA[7] = 1;
                if (CRA[0]) begin
                    irqA <= 0;
                end
            end
            // INTERRUPTS B
            if (!CRB[1] && !CB1) begin
                $display("!CRB[1] && !CB1");
                CRB[7] = 1;
                if (CRB[0]) begin
                    irqB <= 0;
                end
            end
            else if (CRB[1] && CB1) begin
                $display("CRB[1] && CB1");
                CRB[7] = 1;
                if (CRB[0]) begin
                    irqB <= 0;
                end
            end


            // CTRL REGISTER
            CTRLWORD = {RS[1:0], CRA[2]};
            case (CTRLWORD)
                `CNTRL_PREG_A: // Peripherial Reg A
                begin
                    if (rw) begin
                        DO <= PAI;
                        CRA[7] <= 0; // CLEAR INTERRUPT BIT A
                    end
                    else begin
                        PAO <= DI;
                    end
                end
                `CNTRL_PREG_B: // Peripherial Reg B
                begin
                    if (rw) begin
                        DO <= PBI;
                        CRB[7] <= 0; // CLEAR INTERRUPT BIT B
                    end
                    else begin
                        PBO <= DI;
                    end
                end
                `CNTRL_DDR_A: // DDR Reg A
                begin
                    if (rw) begin
                        DO <= DDRA;
                    end
                    else begin
                        DDRA <= DI;
                    end
                end
                `CNTRL_DDR_B: // DDR Reg B
                begin
                    if (rw) begin
                        DO <= DDRB;
                    end
                    else begin
                        DDRB <= DI;
                    end
                end
                `CNTRL_CRA,
                `CNTRL_CRA_ALT: // CRA Reg A
                begin
                    if (rw) begin
                        DO <= CRA;
                    end
                    else begin
                        CRA[5:0] <= DI[5:0];
                    end
                end
                `CNTRL_CRB,
                `CNTRL_CRB_ALT: // CRA Reg B
                begin
                    if (rw) begin
                        DO <= CRB;
                    end
                    else begin
                        CRB[5:0] <= DI[5:0];
                    end
                end

            endcase
        end

    task reset;
        begin
            CRA <= 0;
            CRB <= 0;
            DDRA <= 0;
            DDRB <= 0;
        end
    endtask


endmodule
