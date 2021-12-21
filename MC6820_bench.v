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

    initial begin
      
        reset_n = 0;
        rw = 0;
        enable = 1;
        PAI = 2;
        PBI = 4;
        DI = 8;
        #40
         $display("DI = %d, DO = %d, reset = %d, rw = %d, enable = %d", DI, DO, reset_n, rw, enable );


        reset_n = 1;
        enable = 0;
        RS [1] = 0;
        RS [0] = 1;
        #40
         $display("DI = %d, DO = %d, reset = %d, rw = %d, enable = %d", DI, DO, reset_n, rw, enable );

        rw = 1;
        enable = 1;
        #40
         $display("DI = %d, DO = %d, reset = %d, rw = %d, enable = %d", DI, DO, reset_n, rw, enable );


        rw = 0;
        enable = 0;
        DI = 251;
        #40
         $display("DI = %d, DO = %d, reset = %d, rw = %d, enable = %d", DI, DO, reset_n, rw, enable );

        enable = 1;
        #40
         $display("DI = %d, DO = %d, reset = %d, rw = %d, enable = %d", DI, DO, reset_n, rw, enable );


        rw = 1;
        enable = 0;
        #40
         $display("DI = %d, DO = %d, reset = %d, rw = %d, enable = %d", DI, DO, reset_n, rw, enable );


        enable = 1;
        #40
         $display("DI = %d, DO = %d, reset = %d, rw = %d, enable = %d", DI, DO, reset_n, rw, enable );



        // INTERRUPTS
        reset_n = 0;
        enable = 0;
        CA1 = 0;

        #40
        $display("DI = %d, DO = %d, reset = %d, rw = %d, enable = %d", DI, DO, reset_n, rw, enable );

        reset_n = 1;
        enable = 1;

        #40
         $display("CA1 = %d, IRQA = %d", CA1, irqA );
        $display("DI = %d, DO = %d, reset = %d, rw = %d, enable = %d", DI, DO, reset_n, rw, enable );

      
      
        RS [1] = 1;
      RS [0] = 0;
              CA1 = 1;

        enable = 0;

        #40
         $display("CA1 = %d, IRQA = %d", CA1, irqA );
        $display("DI = %d, DO = %d, reset = %d, rw = %d, enable = %d", DI, DO, reset_n, rw, enable );
   	

        enable = 1;

        #40
         $display("CA1 = %d, IRQA = %d", CA1, irqA );
        $display("DI = %d, DO = %d, reset = %d, rw = %d, enable = %d", DI, DO, reset_n, rw, enable );
   	
      
        $finish;






    end

endmodule
