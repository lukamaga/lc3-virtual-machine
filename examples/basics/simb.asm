        .ORIG x3000

        ; Печатаем приглашение
        LEA   R0, PROMPT
        PUTS

        ; R1 указывает на начало буфера, куда пишем символы
        LEA   R1, BUFFER

READ_LOOP:
        TRAP  x23           ; Считать символ в R0 (GETC)

        ; Проверяем, не нажат ли Enter (CR = x000D)
        LD    R2, CR
        NOT   R2, R2
        ADD   R2, R2, #1
        ADD   R2, R2, R0
        BRz   STOP_READ

        ; Проверяем, не нажат ли LF = x000A (если среда даёт Line Feed)
        LD    R2, LF
        NOT   R2, R2
        ADD   R2, R2, #1
        ADD   R2, R2, R0
        BRz   STOP_READ

        ; Иначе сохраняем символ в буфер
        STR   R0, R1, #0
        ADD   R1, R1, #1
        BR    READ_LOOP

STOP_READ:
        ; Пишем в буфер признак конца строки (нулевой символ)
        AND   R0, R0, #0    
        STR   R0, R1, #0    

        ; Печатаем перевод строки (чтобы отделить вывод)
        LEA   R0, NL_STR
        PUTS

        ; Выводим то, что набрали
        LEA   R0, BUFFER
        PUTS

        TRAP  x25           ; HALT


PROMPT      .STRINGZ "Enter a string: "

; Буфер для введённых символов
BUFFER      .BLKW  40       ; резервируем ~40 слов

CR          .FILL  x000D
LF          .FILL  x000A
NL_STR      .STRINGZ "\n"

        .END
