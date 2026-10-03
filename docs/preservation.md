# Source versions and preservation

This repository collects the saved LC-3 coursework sources and guest programs. The C implementations remain distinguishable, and the exercise sources and guest images retain their original bytes.

## Emulator versions

| Published location | Saved source | Treatment |
| --- | --- | --- |
| [`src/lc3.c`](../src/lc3.c) | Standalone `avm.c` | Primary implementation, imported unchanged before focused corrections |
| [`archive/clion_draft/lc3.c`](../archive/clion_draft/lc3.c) | Earlier `CLionProjects/untitled/main.c` | Exact historical copy; excluded from the default build |
| [`archive/course_vm/VM.c`](../archive/course_vm/VM.c) | `OS/GitHub/OS/VM/VM.c` | Exact historical copy; excluded from the default build |

The standalone source uses the full 16-bit address space and separates the POSIX and Windows include paths correctly. The CLion draft retains its earlier include-guard problem. The separate course VM has keyboard MMIO but allocates only 8,192 words while starting execution at `x3000`, outside that array. Its [README](../archive/course_vm/README.md) explains the mismatch.

The original standalone source is available at [the initial import commit](https://github.com/lukamaga/lc3-virtual-machine/blob/1d09b9da277b775dc5e3a53662ebb46ad1573cf0/src/lc3.c). It has not been replaced in history by the corrected source.

The original eight commits of the separate Git project are preserved on [`archive/course-vm`](https://github.com/lukamaga/lc3-virtual-machine/tree/archive/course-vm), including their recorded authors and dates. Its final historical source is commit `9928df4f7ee3db725a3601131cb7b71b232bb185`. Added archive documentation does not change those commits.

## Focused corrections

The [correction commit](https://github.com/lukamaga/lc3-virtual-machine/commit/2fd7f3a) changes the primary source only:

| Area | Change |
| --- | --- |
| Image byte order | Decode the header and payload explicitly as big-endian bytes |
| Image capacity | Use a size type that can represent all 65,536 memory words, including an origin of zero |
| Image validation | Reject incomplete headers, partial words, oversized payloads, empty payloads and read errors |
| `PUTS` | Emit the low byte of each word, keeping its behavior distinct from packed `PUTSP` |
| String bounds | Stop both string handlers at the end of memory if no terminator was encountered |
| POSIX terminal | Restore settings only after a successful terminal-mode change |
| `JSRR R7` | Read the call target before overwriting R7 with the return address |

These corrections retain the original instruction dispatch, memory layout, console model and fixed entry point. They do not add devices, a guest OS or an exception system. The archived C copies and assembly exercises are unchanged.

## Guest programs

Eight small programs and the separate 2048 example each retain both `.asm` and `.obj` files. The images are executable inputs for the LC-3 guest architecture, so they remain tracked even though host build output is ignored.

Each published assembly file and guest image is byte-identical to its saved counterpart. The names associate sources with images; no reassembly was performed to establish revision equivalence. [The example guide](examples.md) records incomplete exercises and compatibility limits.

## Scope of the archive

The publication includes all distinct coursework VM sources and guest programs found in the inspected local project folders. Identical copies, an empty `VM.c`, IDE state, host executables, debug bundles and generated symbol files are omitted. The external lc3tools distribution, its stock guest OS, unrelated course documents and another student's report are not part of this project.

Source comments and applicable third-party notices are preserved. The tutorial code and 2048 game have separate [license notices](../licenses/README.md); their inclusion does not make them original coursework implementations.

## Verification boundary

Preservation was checked through byte comparisons and SHA-256 hashes. Documentation was compared with the source, the image headers and primary LC-3 references. No compiler, assembler, VM executable or guest program was run as part of preparing this publication. Build and run commands are instructions for readers, not a report of successful runtime validation.

The [file provenance manifest](provenance.json) records the preserved source and image hashes.
