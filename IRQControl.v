`timescale 1us/1us

module IRQControl (clk, readp, deselect, nreset, isLocked);
    input   clk, readp, nreset, deselect;
    output  isLocked;
    reg     isLocked;
    reg     state;
    reg	    next_state;


    parameter NOT_READY = 1'b1, READY = 1'b0;


    always @ (posedge clk, negedge nreset) begin
        if (!nreset) begin
            state <= READY;
        end
        else begin
            state <= next_state;
        end
    end

    always @ (state, readp, deselect) begin
        case (state)
            READY:
                if (readp == 1)
                    next_state = NOT_READY;
                else
                    next_state = READY;
            NOT_READY:
                if (deselect)
                    next_state = READY;
                else
                    next_state = NOT_READY;
            default:
                next_state = READY;
        endcase
    end

    always @ (state, readp, deselect) begin
        case (state)
            READY:
                if (readp == 1)
                    isLocked = 1;
                else
                    isLocked = 0;
            NOT_READY:
                if (deselect)
                    isLocked = 0;
                else
                    isLocked = 1;
            default:
                isLocked = 0;

        endcase
    end
endmodule
