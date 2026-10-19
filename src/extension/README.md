# Runtime extensions

## Runtime behavior

The Linux x86-64 executable and shared-library stubs include `analysis_decoy.c` before they restore the original payload.  
The shared implementation requires no libc.  
A match writes to address 0 and dies from SIGSEGV, so the shell reports exit status 139.  
A shared-library match therefore crashes the entire host process.  

## Process matching

The guard checks visible `/proc/<pid>/comm` names for exact matches:  

- `ida`, `ida64`, `idat`, `idat64`, `idaq`, `idaq64`
- `ghidra`, `ghidraRun`
- `binaryninja`

For a process named `java`, it also checks complete command-line arguments for these exact values:  

- `ghidra.Ghidra`
- `ghidra.GhidraRun`
- `ghidra.app.util.headless.AnalyzeHeadless`

It reads at most 4096 command-line bytes per Java process.  

## Limitations

The check runs once per stub invocation.  
It does not establish that a matching tool is analyzing the target.  
Renamed, hidden, inaccessible, or later-started processes can escape the check.  
The guard skips unavailable procfs entries to preserve existing execution behavior.  
The guard does not modify files, other processes, or system settings.  

## Opaque storage

`upx --opaque` supports Linux and Windows x86-64 executables and shared libraries.  
Other architectures and EFI images are rejected.  

| Format | Opaque payload |
| --- | --- |
| Linux ELF executable or `.so` | Stored blocks are XOR-scrambled instead of compressed. ELF recovery headers stay compressed. |
| Windows PE executable or `.dll` | The preprocessed image is stored XOR-scrambled, then copied back before import and relocation fixups. |

The scramble uses a nonzero byte key derived from the block sizes.  
Linux stored blocks use block method 15, which stock UPX rejects as a corrupt `b_info`.  
Loader-required metadata remains readable, including shared-library relocation inputs and Windows import, export, and TLS tables.  
The runtime stub and `upx -d` reverse the scramble.  
Opaque mode is reversible scrambling, not encryption.  

## Tests

CTest uses synthetic process names and checks the SIGSEGV status for packed executables and `dlopen` callers.  
The synthetic-name tests run serially because their names are visible to other tests.  

[Opaque integration tests](opaque_tests.cmake) compare normal and opaque packing with NRV2B, NRV2D, NRV2E, and LZMA.  
They check executable startup, repeated library loading, exports, initialization, TLS, relocated data, packed executable/library composition, and `upx -t` integrity.  
Linux tests also cover eager and lazy loading and byte-identical recovery of the original input.  
Windows recovery is compared byte-for-byte with normal UPX recovery, which retains existing PE header normalization.  

The [Opaque Runtime Tests workflow](../../.github/workflows/opaque.yml) runs these tests and existing regressions on native Linux and Windows runners for pushes to `devel`.  
Windows runtime acceptance requires a passing Windows job for the corresponding source and generated stubs.  
