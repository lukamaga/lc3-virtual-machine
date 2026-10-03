/*
 * LC-3 Virtual Machine
 * Скомпоновано из материалов https://www.jmeiners.com/lc3-vm/
 *
 * Компиляция (пример для macOS/Linux):
 *    gcc -std=c99 lc3.c -o lc3
 * Запуск (пример):
 *    ./lc3 file.obj
 *
 * Для выхода нажмите Ctrl+C или введите инструкцию HALT (TRAP x25) в загруженной программе.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>

#ifdef _WIN32
  /* --- Windows-specific --- */
  #include <Windows.h>
  #include <conio.h>
#else
  /* --- macOS/Linux --- */
  #include <unistd.h>
  #include <termios.h>
  #include <sys/time.h>
  #include <sys/types.h>
  #include <sys/select.h>
#endif

/* Определяем 16-битное слово, так как LC-3 оперирует 16-битными регистрами и адресами */
typedef uint16_t u16;

/* Размер памяти LC-3: 65536 ячеек по 16 бит */
#define MEMORY_MAX (1 << 16)

/* Память */
static u16 memory[MEMORY_MAX];

/* Регистры (R0-R7), PC, COND, и т.д. */
enum {
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC,    /* program counter */
    R_COND,
    R_COUNT
};

/* Массив регистров */
static u16 reg[R_COUNT];

/* Опкод инструкции */
enum {
    OP_BR = 0,  /* условный переход */
    OP_ADD,     /* сложение */
    OP_LD,      /* загрузка */
    OP_ST,      /* сохранение */
    OP_JSR,     /* переход по подпрограмме */
    OP_AND,     /* логическое И */
    OP_LDR,     /* загрузка с базовым адресом */
    OP_STR,     /* сохранение с базовым адресом */
    OP_RTI,     /* не используется (системная команда) */
    OP_NOT,     /* побитовое НЕ */
    OP_LDI,     /* косвенная загрузка */
    OP_STI,     /* косвенное сохранение */
    OP_JMP,     /* безусловный переход */
    OP_RES,     /* зарезервировано (не использовать) */
    OP_LEA,     /* загрузка адреса */
    OP_TRAP     /* системный вызов */
};

/* Флаги условия */
enum {
    FL_POS = 1 << 0, /* положительный */
    FL_ZRO = 1 << 1, /* ноль */
    FL_NEG = 1 << 2, /* отрицательный */
};

/* TRAP-программы */
enum {
    TRAP_GETC  = 0x20, /* считывает символ (без эхо) */
    TRAP_OUT   = 0x21, /* выводит символ */
    TRAP_PUTS  = 0x22, /* выводит строку */
    TRAP_IN    = 0x23, /* считывает символ (c эхо) */
    TRAP_PUTSP = 0x24, /* выводит строку (по 2 байта) */
    TRAP_HALT  = 0x25  /* остановка */
};

/* Константы */
enum {
    PC_START = 0x3000 /* начальный адрес исполнения (по умолчанию) */
};

/* Утилита: знаковое расширение (bit_count - кол-во значимых бит) */
static u16 sign_extend(u16 x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1)
    {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

/* Утилита: установить флаги условия после записи в регистр */
static void update_flags(u16 r)
{
    if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15)   /* если старший бит 1, число отрицательное */
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}

/* Память: чтение/запись */
static u16 mem_read(u16 address)
{
    return memory[address];
}
static void mem_write(u16 address, u16 val)
{
    memory[address] = val;
}

/* Загрузка образа (.obj) в память */
static void read_image_file(FILE* file)
{
    /* Первые 2 байта — origin (начальный адрес) */
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    /* Меняем порядок байт (LC-3 формат big-endian) */
    origin = (origin << 8) | (origin >> 8);

    /* Читаем оставшиеся слова и записываем в память */
    uint16_t max_read = MEMORY_MAX - origin;
    uint16_t* p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    /* Меняем порядок байт у прочитанного */
    while (read-- > 0)
    {
        *p = (*p << 8) | (*p >> 8);
        p++;
    }
}

static int read_image(const char* image_path)
{
    FILE* file = fopen(image_path, "rb");
    if (!file) { return 0; }
    read_image_file(file);
    fclose(file);
    return 1;
}

/* Проверяем, нажата ли клавиша (не блокируем) */
#ifdef _WIN32
static int check_key()
{
    return kbhit();
}
#else
/* Для UNIX-like (macOS, Linux) */

static struct termios original_tio; /* Сохраняем настройки терминала */

/* Восстанавливаем настройки терминала при выходе */
static void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

/* Отключаем буферизацию (канонический режим, эхо) */
static void disable_input_buffering()
{
    struct termios new_tio;
    tcgetattr(STDIN_FILENO, &original_tio);  /* сохраняем старые настройки */
    new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;      /* выключаем канонический режим и эхо */
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

/* Проверяем, есть ли нажатая клавиша */
static int check_key()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    return select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout) != 0;
}
#endif

/* Сигнал Ctrl+C */
static void handle_interrupt(int signal)
{
    printf("\nОстановка эмуляции LC-3.\n");
    exit(2);
}

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        printf("Использование: %s [образ1.obj] [образ2.obj] ...\n", argv[0]);
        return 2;
    }

    /* Загружаем все .obj в память */
    for (int i = 1; i < argc; ++i)
    {
        if (!read_image(argv[i]))
        {
            fprintf(stderr, "Не удалось загрузить образ: %s\n", argv[i]);
            return 1;
        }
    }

    /* Перехват Ctrl+C */
    signal(SIGINT, handle_interrupt);

#ifndef _WIN32
    /* Отключаем буферизацию stdin (macOS/Linux) */
    disable_input_buffering();
    atexit(restore_input_buffering);
#endif

    /* Инициализация регистров */
    reg[R_PC] = PC_START;
    reg[R_COND] = FL_ZRO;

    int running = 1;
    while (running)
    {
        /* Считываем инструкцию */
        u16 instr = mem_read(reg[R_PC]++);
        u16 op = instr >> 12; /* верхние 4 бита — опкод */

        switch (op)
        {
            case OP_ADD:
            {
                /* ADD DR, SR1, SR2 или ADD DR, SR1, imm5 */
                u16 r0 = (instr >> 9) & 0x7;
                u16 r1 = (instr >> 6) & 0x7;
                u16 imm_flag = (instr >> 5) & 0x1;

                if (imm_flag)
                {
                    /* немедленный операнд */
                    u16 imm5 = sign_extend(instr & 0x1F, 5);
                    reg[r0] = reg[r1] + imm5;
                }
                else
                {
                    /* регистровый операнд */
                    u16 r2 = instr & 0x7;
                    reg[r0] = reg[r1] + reg[r2];
                }
                update_flags(r0);
            }
            break;

            case OP_AND:
            {
                /* AND DR, SR1, SR2 или AND DR, SR1, imm5 */
                u16 r0 = (instr >> 9) & 0x7;
                u16 r1 = (instr >> 6) & 0x7;
                u16 imm_flag = (instr >> 5) & 0x1;

                if (imm_flag)
                {
                    u16 imm5 = sign_extend(instr & 0x1F, 5);
                    reg[r0] = reg[r1] & imm5;
                }
                else
                {
                    u16 r2 = instr & 0x7;
                    reg[r0] = reg[r1] & reg[r2];
                }
                update_flags(r0);
            }
            break;

            case OP_NOT:
            {
                /* NOT DR, SR */
                u16 r0 = (instr >> 9) & 0x7;
                u16 r1 = (instr >> 6) & 0x7;

                reg[r0] = ~reg[r1];
                update_flags(r0);
            }
            break;

            case OP_BR:
            {
                /* BR[nzp] PCoffset */
                u16 n = (instr >> 11) & 1;
                u16 z = (instr >> 10) & 1;
                u16 p = (instr >> 9) & 1;
                u16 pc_offset = sign_extend(instr & 0x1FF, 9);
                if ((n && reg[R_COND] == FL_NEG) ||
                    (z && reg[R_COND] == FL_ZRO) ||
                    (p && reg[R_COND] == FL_POS))
                {
                    reg[R_PC] += pc_offset;
                }
            }
            break;

            case OP_JMP:
            {
                /* JMP BaseR / RET */
                /* для RET BaseR = R7 */
                u16 r1 = (instr >> 6) & 0x7;
                reg[R_PC] = reg[r1];
            }
            break;

            case OP_JSR:
            {
                /* JSR / JSRR */
                u16 long_flag = (instr >> 11) & 1;
                reg[R_R7] = reg[R_PC];
                if (long_flag)
                {
                    u16 long_pc_offset = sign_extend(instr & 0x7FF, 11);
                    reg[R_PC] += long_pc_offset;
                }
                else
                {
                    u16 r1 = (instr >> 6) & 0x7;
                    reg[R_PC] = reg[r1];
                }
            }
            break;

            case OP_LD:
            {
                /* LD DR, PCoffset */
                u16 r0 = (instr >> 9) & 0x7;
                u16 pc_offset = sign_extend(instr & 0x1FF, 9);
                reg[r0] = mem_read(reg[R_PC] + pc_offset);
                update_flags(r0);
            }
            break;

            case OP_LDI:
            {
                /* LDI DR, PCoffset */
                u16 r0 = (instr >> 9) & 0x7;
                u16 pc_offset = sign_extend(instr & 0x1FF, 9);
                /* сначала читаем адрес из памяти... */
                u16 addr = mem_read(reg[R_PC] + pc_offset);
                /* ...затем читаем значение по этому адресу */
                reg[r0] = mem_read(addr);
                update_flags(r0);
            }
            break;

            case OP_LDR:
            {
                /* LDR DR, BaseR, offset6 */
                u16 r0 = (instr >> 9) & 0x7;
                u16 r1 = (instr >> 6) & 0x7;
                u16 offset = sign_extend(instr & 0x3F, 6);
                reg[r0] = mem_read(reg[r1] + offset);
                update_flags(r0);
            }
            break;

            case OP_LEA:
            {
                /* LEA DR, PCoffset */
                u16 r0 = (instr >> 9) & 0x7;
                u16 pc_offset = sign_extend(instr & 0x1FF, 9);
                reg[r0] = reg[R_PC] + pc_offset;
                update_flags(r0);
            }
            break;

            case OP_ST:
            {
                /* ST SR, PCoffset */
                u16 r0 = (instr >> 9) & 0x7;
                u16 pc_offset = sign_extend(instr & 0x1FF, 9);
                mem_write(reg[R_PC] + pc_offset, reg[r0]);
            }
            break;

            case OP_STI:
            {
                /* STI SR, PCoffset */
                u16 r0 = (instr >> 9) & 0x7;
                u16 pc_offset = sign_extend(instr & 0x1FF, 9);
                u16 addr = mem_read(reg[R_PC] + pc_offset);
                mem_write(addr, reg[r0]);
            }
            break;

            case OP_STR:
            {
                /* STR SR, BaseR, offset6 */
                u16 r0 = (instr >> 9) & 0x7;
                u16 r1 = (instr >> 6) & 0x7;
                u16 offset = sign_extend(instr & 0x3F, 6);
                mem_write(reg[r1] + offset, reg[r0]);
            }
            break;

            case OP_TRAP:
            {
                reg[R_R7] = reg[R_PC];
                switch (instr & 0xFF)
                {
                    case TRAP_GETC:
                        /* считываем символ (1 байт) в R0 */
                        reg[R_R0] = (u16)getchar();
                        update_flags(R_R0);
                        break;
                    case TRAP_OUT:
                        /* выводим символ из R0 */
                        putc((char)reg[R_R0], stdout);
                        fflush(stdout);
                        break;
                    case TRAP_PUTS:
                    {
                        /* выводим строку, адрес в R0 */
                        u16* c = memory + reg[R_R0];
                        while (*c)
                        {
                            char ch = (*c) & 0xFF;
                            putc(ch, stdout);
                            char ch2 = (*c) >> 8;
                            if (ch2) putc(ch2, stdout);
                            ++c;
                        }
                        fflush(stdout);
                    }
                    break;
                    case TRAP_IN:
                    {
                        printf("Введите символ: ");
                        fflush(stdout);
                        reg[R_R0] = (u16)getchar();
                        putc((char)reg[R_R0], stdout);
                        fflush(stdout);
                        update_flags(R_R0);
                    }
                    break;
                    case TRAP_PUTSP:
                    {
                        /* выводим строку, адрес в R0; пара символов в одном 16-битном слове */
                        u16* c = memory + reg[R_R0];
                        while (*c)
                        {
                            char ch1 = (*c) & 0xFF;
                            putc(ch1, stdout);
                            char ch2 = (*c) >> 8;
                            if (ch2) putc(ch2, stdout);
                            ++c;
                        }
                        fflush(stdout);
                    }
                    break;
                    case TRAP_HALT: 
                        printf("HALT\n");
                        fflush(stdout);
                        running = 0;
                        break;
                }
            }
            break;

            case OP_RES:
            case OP_RTI:
            default:
                /* Зарезервировано или не используется */
                printf("Неизвестная или зарезервированная команда (0x%X). Остановка.\n", op);
                running = 0;
                break;
        }
    }

    return 0;
}
