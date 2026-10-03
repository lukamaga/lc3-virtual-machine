# Earlier CLion draft

`lc3.c` preserves the earlier local `CLionProjects/untitled/main.c` byte for byte. It is an alternate draft of the tutorial-based emulator retained for historical context. Use [`src/lc3.c`](../../src/lc3.c) from the repository root as the primary implementation.

This draft is not part of the default build. Its platform include block places POSIX headers inside `#ifdef _WIN32`, while the POSIX terminal implementation is selected on the opposite branch. The later standalone source corrects this arrangement. The draft also retains the original image-loader and string-output limitations described in the repository documentation.

The source header credits [Write your Own Virtual Machine](https://www.jmeiners.com/lc3-vm/). This archived copy retains that attribution and its original comments. It is not a separate original implementation or a claim of full LC-3 conformance.

Original SHA-256: `c3b3643a766cfc7b524c8695208cf19cb85e831e552c70cf3e6168ebe143f0c7`.
