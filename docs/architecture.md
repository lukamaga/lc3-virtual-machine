# Architecture and image format

This guide describes [`src/lc3.c`](../src/lc3.c). The archived C versions have different memory and terminal behavior; their own READMEs describe those differences.

## Machine state

Memory is an array of 65,536 unsigned 16-bit words, addressed from `x0000` to `xFFFF`. Each address selects one word, not one byte. Eight general-purpose registers, the program counter and condition flags hold the processor state.

The fetch loop reads `memory[PC]`, increments PC by one word, then extracts the opcode with `instruction >> 12`. Register writes use 16-bit storage, so values wrap modulo 65,536. N/Z/P flags distinguish a negative value by bit 15, zero, and a positive nonzero value.

The implementation updates condition flags after `ADD`, `AND`, `NOT`, `LD`, `LDI`, `LDR` and `LEA`. Its `GETC` and `IN` handlers also update them. Stores, branches and return-link writes do not.

## Instructions

| Group | Instructions | Purpose |
| --- | --- | --- |
| Arithmetic and logic | `ADD`, `AND`, `NOT` | Register arithmetic, masked values and bitwise complement |
| PC-relative addressing | `LD`, `ST`, `LEA` | PC-relative loads, stores and address calculation |
| Indirect addressing | `LDI`, `STI` | Read a PC-relative pointer, then access its target |
| Base addressing | `LDR`, `STR` | Access memory relative to a base register |
| Control flow | `BR`, `JMP`, `RET` | Conditional branches and register-based jumps |
| Subroutines | `JSR`, `JSRR` | Save the next PC in R7 and enter a subroutine |
| Console interface | `TRAP` | Dispatch supported vectors to host C functions |

`RET` is the `JMP R7` encoding. `JSR` and `JSRR` share an opcode. Fourteen opcode values have implemented handlers; `RTI` and the reserved opcode stop the interpreter with a message.

### Signed operands

| Field | Width | Signed range | Used by |
| --- | --- | --- | --- |
| Immediate | 5 bits | -16 to 15 | `ADD`, `AND` |
| Base offset | 6 bits | -32 to 31 | `LDR`, `STR` |
| PC offset | 9 bits | -256 to 255 | `BR`, `LD`, `ST`, `LDI`, `STI`, `LEA` |
| Call offset | 11 bits | -1024 to 1023 | `JSR` |

Offsets are sign-extended before use. PC-relative targets are calculated from the incremented PC:

```text
target = (next_PC + sign_extended_offset) mod 65536
```

For `JSRR`, the base register is read before R7 is replaced by the return address. This matters when the instruction itself uses R7 as the base register.

## Console TRAP handlers

The implementation saves the next PC in R7 and handles the following vectors directly. It does not dispatch through a guest trap-vector table.

| Vector | Name | Behavior |
| --- | --- | --- |
| `x20` | `GETC` | Read a character into R0 without explicit handler echo |
| `x21` | `OUT` | Write the low character byte of R0 |
| `x22` | `PUTS` | Starting at the address in R0, write one low-byte character per word until a zero word |
| `x23` | `IN` | Print a prompt, read and echo a character, and store it in R0 |
| `x24` | `PUTSP` | Write the low byte, then a nonzero high byte, of successive words until a zero word |
| `x25` | `HALT` | Print `HALT` and leave the execution loop |

The two string handlers also stop at the end of the memory array. An unterminated string is truncated there without a guest exception. Other TRAP vectors are ignored after the return address is saved.

Input uses the host C library. The POSIX branch attempts to disable canonical input and echo in an interactive terminal; the Windows branch does not perform the equivalent mode change. End-of-file currently becomes `xFFFF` in R0. There is no separate input-EOF policy.

## Binary image layout

An LC-3 image contains big-endian 16-bit words:

```text
Byte offsets     Contents
0, 1             Load origin, high byte first
2, 3             First word, loaded at origin
4, 5             Second word, loaded at origin + 1
...              Remaining instruction and data words
```

The origin word is a file header, not a word copied into guest memory. The loader decodes bytes explicitly, so its interpretation does not depend on host byte order.

The primary implementation accepts a complete header followed by at least one complete payload word. It rejects a missing or partial header, an odd payload byte count, a payload extending past `xFFFF`, a header-only image and stream read errors. At origin `x0000`, 65,536 words fit; at origin `xFFFF`, one word fits.

Every image is loaded before execution starts. Later image arguments may overwrite earlier memory. A load failure exits before guest execution; execution otherwise starts at `x3000`.

## Historical device mappings

The primary emulator performs ordinary array reads and has no keyboard MMIO. Its saved `check_key` helper is unused.

The [separate course VM](../archive/course_vm/README.md) contains keyboard status/data handling at `x1E00` and `x1E02`. The saved 2048 guest uses those same addresses. That archive also retains a memory-capacity mismatch, so it should not be treated as a ready alternative for running the game.

## Further reading

- [LC-3 ISA reference](https://www.jmeiners.com/lc3-vm/supplies/lc3-isa.pdf)
- [Tutorial implementation](https://www.jmeiners.com/lc3-vm/)
- [lc3tools instruction definitions](https://github.com/haplesshero13/lc3tools/blob/master/lc3.def), including the `JSRR R7` operand-order note
- [lc3as source](https://github.com/haplesshero13/lc3tools/blob/master/lc3.f), including big-endian word output and origin handling
