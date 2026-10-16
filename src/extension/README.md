# Linux x86-64 runtime decoy

The executable and shared-library stubs include `analysis_decoy.c` before they restore the original payload. The shared implementation requires no libc. A match prints `Hello World` and calls `exit_group(0)`. A shared-library match therefore terminates the entire host process.

The guard checks visible `/proc/<pid>/comm` names for exact matches: `ida`, `ida64`, `idat`, `idat64`, `idaq`, `idaq64`, `ghidra`, `ghidraRun`, and `binaryninja`. For a process named `java`, it also checks complete command-line arguments for `ghidra.Ghidra`, `ghidra.GhidraRun`, or `ghidra.app.util.headless.AnalyzeHeadless`. It reads at most 4096 command-line bytes per Java process.

The check runs once per stub invocation. It does not establish that a matching tool is analyzing the target. Renamed, hidden, inaccessible, or later-started processes can escape the check. Unavailable procfs entries are skipped to preserve existing execution behavior. No files, other processes, or system settings are modified.

This changes runtime behavior only. It does not prevent ordinary `upx -d`, change the compression format, or replace the stored payload with a separate Hello World ELF. Shared libraries can expose uncompressed loader metadata before their initialization stub runs.

CTest uses synthetic process names and checks exact output and exit status for packed executables and `dlopen` callers. The synthetic-name tests run serially because their names are visible to other tests. All new C sources reside in this directory.
