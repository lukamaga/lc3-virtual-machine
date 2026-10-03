# Archived LC-3 coursework history

This branch preserves the eight original commits of the separate coursework VM, with their recorded authors and dates. The final historical source is `9928df4f7ee3db725a3601131cb7b71b232bb185`. This documentation commit adds no source changes.

The collected project, guest programs and build instructions are on [main](https://github.com/lukamaga/lc3-virtual-machine). The same final archived source is included there under [archive/course_vm](https://github.com/lukamaga/lc3-virtual-machine/tree/main/archive/course_vm).

## Historical implementation

[`VM/VM.c`](VM/VM.c) allocates 8,192 memory words but starts execution at `x3000`, outside that array. Its saved guest images also use that origin. This memory-capacity mismatch is retained as historical source; use the primary implementation on main for the documented build.

The source implements keyboard polling at `x1E00` and `x1E02`. The archived 2048 game uses those custom addresses, while the primary implementation on main does not provide keyboard MMIO. Neither fact makes this snapshot a validated game runner.

## Reference code

This implementation follows [Write your Own Virtual Machine](https://www.jmeiners.com/lc3-vm/) by Justin Meiners and Ryan Pendleton. The tutorial code's complete [MIT notice](LICENSE.tutorial.txt) is retained on this branch. The tutorial article and figures are not included.
