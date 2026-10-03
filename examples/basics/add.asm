       .ORIG x3000
        LD   R1, TWO
        LD   R2, THREE
        ADD  R0, R1, R2
        LD   R4, ASCII0
        ADD  R0, R0, R4
        LEA  R1, SUM_MSG
        PUTS
        OUT
        TRAP x25

TWO     .FILL x0002
THREE   .FILL x0003
ASCII0  .FILL x0030
SUM_MSG .STRINGZ "SUM: "
        .END