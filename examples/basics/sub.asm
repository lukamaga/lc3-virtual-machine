        .ORIG x3000
        LD   R1, SEVEN
        LD   R2, FOUR
        NOT  R2, R2
        ADD  R2, R2, #1
        ADD  R0, R1, R2
        LD   R4, ASCII0
        ADD  R0, R0, R4
        LEA  R1, DIFF_MSG
        PUTS
        OUT
        TRAP x25

SEVEN   .FILL x0007
FOUR    .FILL x0004
ASCII0  .FILL x0030
DIFF_MSG .STRINGZ "DIFF: "
        .END
