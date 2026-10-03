        .ORIG x3000
        LEA   R0, PROMPT
        PUTS
        TRAP  x23
        ST    R0, RESULT
        LEA   R0, RESPONSE
        PUTS
        LD    R0, RESULT
        OUT
        TRAP  x25
PROMPT   .STRINGZ "Enter a character: "
RESPONSE .STRINGZ "You entered: "
RESULT   .BLKW 1
        .END
