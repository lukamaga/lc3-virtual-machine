# LC-3 guest programs

The examples cover arithmetic, branching, addressing modes, console input and subroutines. Each `.asm` file has a same-named saved `.obj` image. These images contain LC-3 machine words and are inputs to the emulator, not native executables for the host computer.

All nine saved images have origin `x3000`, matching the primary emulator's initial program counter. Source and image pairs share their original filenames; the images have not been reassembled to establish that each one matches the saved assembly exactly.

## Small programs

The eight programs in [`examples/basics`](../examples/basics/) are coursework experiments. Their original names are retained, including names that describe an intended exercise rather than the implemented behavior.

| Program | Concepts and actual source behavior | Details to keep in mind |
| --- | --- | --- |
| [`input.asm`](../examples/basics/input.asm) · [`input.obj`](../examples/basics/input.obj) | Displays a prompt, reads one character with `TRAP x23`, stores it, displays a response and outputs the stored character. | `x23` is `IN`, which adds its own prompt and echo. The program's prompt does not replace that built-in behavior. |
| [`ld.asm`](../examples/basics/ld.asm) · [`ld.obj`](../examples/basics/ld.obj) | Demonstrates `LEA`, base-plus-offset `LDR`, indirect `LDI` and PC-relative `LD` to obtain string addresses for `PUTS`. | Strings that mention a subroutine are data labels; this example does not call a subroutine. |
| [`add.asm`](../examples/basics/add.asm) · [`add.obj`](../examples/basics/add.obj) | Adds 2 and 3, then converts the sum to an ASCII digit in `R0`. | Loads the message address into `R1`, but `PUTS` reads its address from `R0`. The message-output sequence therefore uses the digit value as a memory address. |
| [`sub.asm`](../examples/basics/sub.asm) · [`sub.obj`](../examples/basics/sub.obj) | Forms the two's complement of 4 and adds it to 7, then converts the result to an ASCII digit. | Has the same `R1`/`R0` message-address mismatch as `add.asm`. |
| [`mul.asm`](../examples/basics/mul.asm) · [`mul.obj`](../examples/basics/mul.obj) | Starts with 5 and 7, repeatedly decrements each nonzero value, then outputs the remaining values. | Despite its filename, the source does not implement multiplication. |
| [`loop.asm`](../examples/basics/loop.asm) · [`loop.obj`](../examples/basics/loop.obj) | Decrements a counter initialized to 5 and branches until it reaches zero. | Builds the ASCII digit in `R2` and loads a newline into `R3`, but both `OUT` calls read `R0`. |
| [`simb.asm`](../examples/basics/simb.asm) · [`simb.obj`](../examples/basics/simb.obj) | Reads characters until CR or LF, writes a zero terminator and displays the buffered string. | Uses `IN` for every character, despite a `GETC` comment. The 40-word buffer has no bounds check; only 39 non-terminating characters fit while leaving room for the zero terminator. |
| [`sum_large.asm`](../examples/basics/sum_large.asm) · [`sum_large.obj`](../examples/basics/sum_large.obj) | Adds 1234 and 567 and attempts decimal conversion with `DECOUT` and `PRINT_DIGIT` subroutines. | Return addresses in `R7` are overwritten by nested `JSR` and scratch calculations. Its subtraction step adds the computed difference back to the previous value, and a digit check branches on the subtraction flags. This is an unfinished decimal-output experiment. |

These descriptions come from the assembly. They are not recorded execution results or a claim that every example completes correctly.

## Start with character input

After building the emulator as described in the [main README](../README.md), run the saved single-character example in an interactive terminal:

```sh
./build/lc3 examples/basics/input.obj
```

Enter one character. The program uses the VM's console `IN`, `PUTS` and `OUT` handlers and then reaches `HALT`. Terminal behavior depends on the platform implementation; see the main README's limitations.

The addressing example is another compact program to inspect:

```sh
./build/lc3 examples/basics/ld.obj
```

Neither example needs the memory-mapped keyboard registers used by the 2048 game.

## Assembly source and object images

An LC-3 assembler such as `lc3as` is needed only when editing or reassembling the guest source. A C compiler cannot assemble these files. To keep a newly assembled image separate from the saved image, work on a copy:

```sh
mkdir -p build/examples
cp examples/basics/input.asm build/examples/input.asm
lc3as build/examples/input.asm
./build/lc3 build/examples/input.obj
```

The assembler is an external dependency. Its toolchain distribution is not part of this repository.

## Third-party 2048 example

[`examples/2048`](../examples/2048/) contains Ryan Pendleton's LC-3 game, its original copyright header and the saved guest image. The local assembly uses keyboard status/data addresses `x1E00` and `x1E02`. The primary VM does not implement keyboard polling through these addresses, so the game is not a supported demo for that executable. See its [compatibility and attribution notes](../examples/2048/README.md).
