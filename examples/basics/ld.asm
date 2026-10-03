        .ORIG x3000

        ; LEA
        LEA   R0, MSG
        PUTS

        ; LDR
        LEA   R2, TABLE
        LDR   R1, R2, #0
        ADD   R0, R1, #0
        PUTS

        ; LDI
        LDI   R1, PTR_H2
        ADD   R0, R1, #0
        PUTS

        ; LD
        LD    R1, PTR_H2_2
        ADD   R0, R1, #0
        PUTS

        TRAP  x25

MSG     .STRINGZ "Program 2 Start\n"
HELLO   .STRINGZ "Hello from subroutine 1\n"
HELLO2  .STRINGZ "Hello from subroutine 2\n"

TABLE   .FILL HELLO

PTR_H2      .FILL H2_ADDR
H2_ADDR     .FILL HELLO2

PTR_H2_2    .FILL HELLO2

        .END
