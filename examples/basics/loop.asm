.ORIG x3000
LD   R0, COUNT
LOOP    ADD  R0, R0, #-1
        BRz  DONE
        LD   R1, ASCII0
        ADD  R2, R0, R1
        OUT
        LD   R3, NEWLINE
        OUT
        BRp  LOOP
DONE    TRAP x25
COUNT   .FILL x0005
ASCII0  .FILL x0030
NEWLINE .FILL x000A
.END
