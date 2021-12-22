`timescale 1ns/1ps

module MC6820_bench();
    reg [ 7:0] DI;		// DATA

    reg [ 7:0] PAI;		// PA

    reg [ 7:0] PBI;		// PB

    reg CA1;		// CHIP SELECT
    reg CB1;		// CHIP SELECT


    reg [ 2:0] CS;		// CHIP SELECT
    reg [ 1:0] RS;		// REG SELECT

    reg rw;				// 0 = READ
    reg enable;
    reg reset_n;			// LOW will reset
    wire irqA;			// LOW will reset
    wire irqB;			// LOW will reset
    reg CA2I;			// LOW will reset
    reg CB2I;			// LOW will reset


    wire [ 7:0] DO;
    wire [ 7:0] PAO;
    wire [ 7:0] PBO;
    wire CA2O;
    wire CB2O;

    MC6820 myMC6820(
               DI,
               DO,
               PAI,
               PAO,
               PBI,
               PBO,
               CA1,
               CB1,
               CA2I,
               CA2O,
               CB2I,
               CB2O,
               CS,
               RS,
               rw,
               enable,
               reset_n,
               irqA,
               irqB);

    parameter HALF_PERIOD = 10;
    parameter PERIOD = HALF_PERIOD*2;

    initial begin
        enable = 1'b0;
        forever begin
            #HALF_PERIOD enable = ~enable;
        end
    end

    // Generate the reset
    initial begin
        reset_n = 1'b1;
        #HALF_PERIOD
         reset_n = 1'b0;
        #HALF_PERIOD
         reset_n = 1'b1;
    end

    initial begin
        $monitor("t=%3d enable=%d, reset_n=%d, irqA=%d, CA1=%d DI=%b, DO=%b RS=%b\n ---------------",$time,enable, reset_n, irqA, CA1, DI, DO, RS );
    end


    initial begin
        rw = 0;
        CA1 = 1'b1;
        PAI=8'b11111111;

        #PERIOD
         #PERIOD


         // INTERRUPTS
         $display("1 READ REG A in DO");
        rw=1;
        RS = 2'b01;
        #PERIOD

         $display("2 WRITE DI to REGA");
        rw=0;
        RS = 2'b01;
        DI = 5'b00001;
        #PERIOD

         $display("3 READ REG A in DO");
        rw=1;
        RS = 2'b01;
        #PERIOD


         $display("4 TRIGGER CA1");
        rw=1;
        CA1=0;
        #PERIOD

         $display("5 DETRIGGER CA1");
        rw=1;
        CA1=1;
        #PERIOD


         $display("6 READ REG A in DO");
        rw=1;
        RS = 2'b01;
        #PERIOD

         $display("7 WRITE DI to REGA SET PERIFERIAL");
        rw=0;
        RS = 2'b01;
        DI = 5'b00101;
        #PERIOD

         $display("8 READ PERIFERIAL A in DO");
        rw=1;
        RS = 2'b00;
        #PERIOD

         $display("9 READ REG A in DO");
        rw=1;
        RS = 2'b01;
        #PERIOD
         $finish;

    end

endmodule
