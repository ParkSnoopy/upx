# Linux x86-64 runtime decoy

## Runtime behavior

The executable and shared-library stubs include `analysis_decoy.c` before they restore the original payload.  
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

`upx --opaque` supports Linux x86-64 executables only.  
It stores each payload block instead of compressing it, except for the first block (ELF headers), which stays compressed.  
It then XORs the stored bytes with a key derived from the block sizes.  
Stored blocks use block method 15, which stock UPX rejects as a corrupt `b_info`.  
Patching the method byte alone yields scrambled bytes and a checksum failure.  
The stored payload keeps the file's entropy close to the input's.  
This fork reverses the scramble during `upx -d`.  
Shared libraries can expose uncompressed loader metadata before their initialization stub runs.  

## Tests

CTest uses synthetic process names and checks the SIGSEGV status for packed executables and `dlopen` callers.  
It also packs an executable with `--opaque`, runs it normally, triggers the decoy, and checks the `upx -d` round-trip.  
The synthetic-name tests run serially because their names are visible to other tests.  
All new C sources reside in this directory.  
