# Archived course VM variant

`VM.c` preserves a separate historical version of the LC-3 emulator byte for byte. The default implementation is [`src/lc3.c`](../../src/lc3.c); this archived file is not part of its build.

The preserved source allocates 8,192 memory words (`MEMORY_MAX = 1 << 13`) but starts execution at `0x3000`, beyond that array. The saved guest images also use origin `0x3000`. This mismatch is part of the historical source and remains unchanged here. Its keyboard registers use the custom addresses `0x1E00` and `0x1E02`, which also appear in the archived 2048 guest program.

The [original eight-commit history](https://github.com/lukamaga/lc3-virtual-machine/tree/archive/course-vm) is preserved on the separate `archive/course-vm` branch. Its final historical source is commit `9928df4f7ee3db725a3601131cb7b71b232bb185`. Existing commit authors and dates are retained, followed by a new commit containing archive documentation and the tutorial license notice.

This emulator follows the structure of [Write your Own Virtual Machine](https://www.jmeiners.com/lc3-vm/). See the repository attribution and third-party notices for the tutorial's code license.

Original SHA-256: `3ba53f3d056f7509ddfe435ed30f2da1ea0853d89840394ce9df17d4356c9986`.
