# 2048 for LC-3

This is the LC-3 implementation of 2048 by **Ryan Pendleton**, created in December 2014. The assembly's original project, author and license header is retained. It is included as a third-party guest program used alongside the coursework VM.

| File | Contents |
| --- | --- |
| [`2048.asm`](2048.asm) | The saved assembly source, including the local keyboard-address adaptation. |
| [`2048.obj`](2048.obj) | The original saved LC-3 image, with origin `x3000`. |

The assembly and image are paired by their saved filename. No claim is made that the complete image was rebuilt from this exact source.

## Keyboard compatibility

The local assembly polls memory-mapped keyboard registers using these constants:

```asm
OS_KBSR     .FILL x1E00
OS_KBDR     .FILL x1E02
```

The saved image also contains these values at the corresponding locations. They differ from the standard LC-3 keyboard addresses `xFE00` and `xFE02`.

The primary VM implements character input through console TRAP handlers but does not supply keyboard status/data registers at `x1E00` and `x1E02`. Consequently, this game is not a supported demo for the primary executable. Loading it does not establish that its keyboard loop can receive input.

For an introductory console program, use the [single-character input example](../basics/input.asm) and the [example guide](../../docs/examples.md).

## Attribution and license

Upstream project: [rpendleton/lc3-2048](https://github.com/rpendleton/lc3-2048).

The game is distributed under the MIT License, copyright 2014 Ryan Pendleton. The complete original notice is in [`licenses/lc3-2048-MIT.txt`](../../licenses/lc3-2048-MIT.txt), copied from the [upstream license](https://github.com/rpendleton/lc3-2048/blob/main/LICENSE). The local keyboard-address adaptation is documented above; the game itself is not claimed as an original coursework implementation.
