#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>

#include <Windows.h>
#include <conio.h>

/* 16 bit word */
typedef uint16_t u16;

/* 8192 16 bit memory blocks */
#define MEMORY_MAX (1 << 13)

/* memory */
static u16 memory[MEMORY_MAX];

/* registers */
enum 
{
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC,
    R_IR,
    R_COND,
    R_COUNT
};

enum
{
    MR_KBSR = 0xFE00, /* keyboard status */
    MR_KBDR = 0xFE02  /* keyboard data */
};

/* register array */
static u16 reg[R_COUNT];

/* opcodes */
enum 
{
    OP_BR = 0,  
    OP_ADD,     
    OP_LD,      
    OP_ST,      
    OP_JSR,     
    OP_AND,     
    OP_LDR,     
    OP_STR,     
    OP_RTI,     /* not used (system return to interrupt) */
    OP_NOT,     
    OP_LDI,    
    OP_STI,     
    OP_JMP,     
    OP_LDPTR,     /* not used (system load PTR) */
    OP_LEA,     
    OP_TRAP     
};

/* conditional flags */
enum 
{
    FL_POS = 1 << 0, 
    FL_ZRO = 1 << 1, 
    FL_NEG = 1 << 2, 
};

enum {
    PC_START = 0x3000 /* initial execution address */
};

enum
{
    TRAP_GETC = 0x20,
    TRAP_OUT = 0x21,
    TRAP_PUTS = 0x22,  /* output a word string */
    TRAP_IN = 0x23,    /* get character from keyboard, echoed onto the terminal */
    TRAP_PUTSP = 0x24, /* output a byte string */
    TRAP_HALT = 0x25
};

HANDLE hStdin = INVALID_HANDLE_VALUE;
DWORD fdwMode, fdwOldMode;

void disable_input_buffering()
{
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &fdwOldMode); /* save old mode */
    fdwMode = fdwOldMode
            ^ ENABLE_ECHO_INPUT  /* no input echo */
            ^ ENABLE_LINE_INPUT; /* return when one or
                                    more characters are available */
    SetConsoleMode(hStdin, fdwMode); /* set new mode */
    FlushConsoleInputBuffer(hStdin); /* clear buffer */
}

void restore_input_buffering()
{
    SetConsoleMode(hStdin, fdwOldMode);
}

u16 check_key()
{
    return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
}

/* Ctrl+C */
void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

static u16 sign_extend(u16 x, int bit_count)
{
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

static void update_flags(u16 r)
{
    if (reg[r] == 0)
    {
        reg[R_COND] = FL_ZRO;
    }
    else if (reg[r] >> 15)
    {
        reg[R_COND] = FL_NEG;
    }
    else
    {
        reg[R_COND] = FL_POS;
    }
}

u16 mem_read(u16 address)
{
    if (address == MR_KBSR)
    {
        if (check_key())
        {
            memory[MR_KBSR] = (1 << 15);
            memory[MR_KBDR] = getchar();
        }
        else
        {
            memory[MR_KBSR] = 0;
        }
    }
    return memory[address];
}

void mem_write(u16 address, u16 val)
{
    memory[address] = val;
}

u16 swap16(u16 x)
{
    return (x << 8) | (x >> 8);
}

void read_image_file(FILE* file)
{
    u16 origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin);

    u16 max_read = MEMORY_MAX - origin;
    u16* p = memory + origin;
    size_t read = fread(p, sizeof(u16), max_read, file);

    while (read-- > 0)
    {
        *p = swap16(*p);
        ++p;
    }
}

int read_image(const char* image_path)
{
    FILE* file = fopen(image_path, "rb");
    if (!file) { return 0; };
    read_image_file(file);
    fclose(file);
    return 1;
}

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        printf("lc3 [image-file1] ...\n");
        exit(2);
    }
    
    for (int j = 1; j < argc; ++j)
    {
        if (!read_image(argv[j]))
        {
            printf("failed to load image: %s\n", argv[j]);
            exit(1);
        }
    }

    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    reg[R_COND] = FL_ZRO;
    reg[R_PC] = PC_START;

    int running = 1;
    while (running)
    {
        u16 instr = mem_read(reg[R_PC]++);
        u16 op = instr >> 12; 

        switch (op)
        {
            case OP_ADD:
            {
                //destination register
                u16 r0 = (instr >> 9) & 0x7;
                //source register
                u16 r1 = (instr >> 6) & 0x7;
                //immediate flag
                u16 imm_flag = (instr >> 5) & 0x1;

                if (imm_flag)
                {
                    //immediate value
                    u16 imm5 = sign_extend(instr & 0x1F, 5);
                    reg[r0] = reg[r1] + imm5;
                }
                else
                {
                    //source register 2
                    u16 r2 = instr & 0x7;
                    reg[r0] = reg[r1] + reg[r2];
                }

                update_flags(r0);
            }
            break;

            case OP_AND:
            {
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
                u16 r0 = (instr >> 9) & 0x7;
                u16 r1 = (instr >> 6) & 0x7;
            
                reg[r0] = ~reg[r1];
                update_flags(r0);
            }
            break;

            case OP_BR:
            {
                u16 pc_offset = sign_extend(instr & 0x1FF, 9);
                u16 cond_flag = (instr >> 9) & 0x7;
                if (cond_flag & reg[R_COND])
                {
                    reg[R_PC] += pc_offset;
                }
            }
            break;

            case OP_JMP:
            {
                u16 r1 = (instr >> 6) & 0x7;
                reg[R_PC] = reg[r1];
            }
            break;

            case OP_JSR:
            {
                u16 long_flag = (instr >> 11) & 1;
                reg[R_R7] = reg[R_PC];
                if (long_flag)
                {
                    u16 long_pc_offset = sign_extend(instr & 0x7FF, 11);
                    reg[R_PC] += long_pc_offset;  /* JSR */
                }
                else
                {
                    u16 r1 = (instr >> 6) & 0x7;
                    reg[R_PC] = reg[r1]; /* JSRR */
                }
            }
            break;

            case OP_LD:
            {
                u16 r0 = (instr >> 9) & 0x7;
                u16 pc_offset = sign_extend(instr & 0x1FF, 9);
                reg[r0] = mem_read(reg[R_PC] + pc_offset);
                update_flags(r0);
            }
            break;

            case OP_LDI:
            {
                //destination register
                u16 r0 = (instr >> 9) & 0x7;
                //PC offset
                u16 pc_offset = sign_extend(instr & 0x1FF, 9);
                reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset));
                update_flags(r0);
            }
            break;

            case OP_LDR:
            {
                u16 r0 = (instr >> 9) & 0x7;
                u16 r1 = (instr >> 6) & 0x7;
                u16 offset = sign_extend(instr & 0x3F, 6);
                reg[r0] = mem_read(reg[r1] + offset);
                update_flags(r0);
            }
            break;

            case OP_LEA:
            {
                u16 r0 = (instr >> 9) & 0x7;
                u16 pc_offset = sign_extend(instr & 0x1FF, 9);
                reg[r0] = reg[R_PC] + pc_offset;
                update_flags(r0);
            }
            break;

            case OP_ST:
            {
                u16 r0 = (instr >> 9) & 0x7;
                u16 pc_offset = sign_extend(instr & 0x1FF, 9);
                mem_write(reg[R_PC] + pc_offset, reg[r0]);
            }
            break;

            case OP_STI:
            {
                u16 r0 = (instr >> 9) & 0x7;
                u16 pc_offset = sign_extend(instr & 0x1FF, 9);
                mem_write(mem_read(reg[R_PC] + pc_offset), reg[r0]);
            }
            break;

            case OP_STR:
            {
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
                    {
                        reg[R_R0] = (u16)getchar();
                        update_flags(R_R0);
                    }
                    break;
                    case TRAP_OUT:
                    {
                        putc((char)reg[R_R0], stdout);
                        fflush(stdout);
                    }
                    break;
                    case TRAP_PUTS:
                    {
                        u16* c = memory + reg[R_R0];
                        while (*c)
                        {
                            putc((char)*c, stdout);
                            ++c;
                        }
                        fflush(stdout);
                    }
                    break;
                    case TRAP_IN:
                    {
                        printf("Enter a character: ");
                        char c = getchar();
                        putc(c, stdout);
                        fflush(stdout);
                        reg[R_R0] = (u16)c;
                        update_flags(R_R0);
                    }
                    break;
                    case TRAP_PUTSP:
                    {
                        u16* c = memory + reg[R_R0];
                        while (*c)
                        {
                            char char1 = (*c) & 0xFF;
                            putc(char1, stdout);
                            char char2 = (*c) >> 8;
                            if (char2) putc(char2, stdout);
                            ++c;
                        }
                        fflush(stdout);
                    }
                    break;
                    case TRAP_HALT:
                    {
                        puts("HALT");
                        fflush(stdout);
                        running = 0;
                    }
                    break;
                }
            }
            break;

            case OP_LDPTR:
            {
                
            }
            break;
            case OP_RTI:
            default:
                abort();
                break;
        }
    }
    restore_input_buffering();
    return 0;
}