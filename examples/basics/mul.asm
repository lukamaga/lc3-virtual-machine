        .ORIG x3000
        LD   R1, NUM1
        LD   R2, NUM2
LOOP    ADD  R3, R1, R2
        BRz  DONE
        AND  R4, R1, R1
        BRz  SKIP1
        ADD  R1, R1, #-1
SKIP1   AND  R4, R2, R2
        BRz  SKIP2
        ADD  R2, R2, #-1
SKIP2   BR   LOOP
DONE    LD   R4, ASCII0
        ADD  R0, R1, R4
        OUT
        LD   R0, SPACE
        OUT
        ADD  R0, R2, R4
        OUT
        LD   R0, NEWLINE
        OUT
        TRAP x25
NUM1    .FILL x0005
NUM2    .FILL x0007
ASCII0  .FILL x0030
SPACE   .FILL x0020
NEWLINE .FILL x000A
        .END
