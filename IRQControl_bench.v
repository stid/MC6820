`timescale 1us/1us

module IRQControl_bench();
    wire isLocked;		// DATA

    reg clk;
    reg readp, deselect;
    reg nreset;

    IRQControl IRQAControl(
	  clk,
      readp,
      deselect,
      nreset,
      isLocked
    );

    parameter HALF_PERIOD = 5;
    parameter PERIOD = HALF_PERIOD*2;

    initial begin
        clk = 1'b0;
        forever begin
            #HALF_PERIOD clk = ~clk;
        end
    end


    initial begin
      $monitor("#: %d, clk: %d, readp: %d, deselect: %d, isLocked: %d, nreset: %d",$time, clk, readp, deselect, isLocked, nreset);
    end

  

    initial begin
        nreset = 1'b1;
      	readp=0;
      	deselect=0;
       #PERIOD
         nreset = 1'b0;
       #PERIOD
         nreset = 1'b1;
      $display("START");
      
      
      	readp=1;
        #PERIOD
      	readp=0;
        #PERIOD
      	deselect=1;
        #PERIOD
      	deselect=0;
        #PERIOD
     
      
         $finish;

    end

endmodule
