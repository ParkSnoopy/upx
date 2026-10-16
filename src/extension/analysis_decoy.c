/* Shared by the Linux x86-64 executable and shared-library stubs. */

#if defined(__x86_64__)

static long
analysis_syscall(long number, long arg1, long arg2, long arg3)
{
    long result;
    __asm__ volatile ("syscall"
        : "=a" (result)
        : "0" (number), "D" (arg1), "S" (arg2), "d" (arg3)
        : "rcx", "r11", "cc", "memory");
    return result;
}

static int
analysis_name_equal(char const *name, unsigned length, char const *expected)
{
    unsigned i = 0;
    while (i < length && expected[i] && name[i] == expected[i]) ++i;
    return i == length && !expected[i];
}

static int
analysis_tool_name(char const *name, unsigned length)
{
    char const *names[] = {
        addr_string("ida"), addr_string("ida64"),
        addr_string("idat"), addr_string("idat64"),
        addr_string("idaq"), addr_string("idaq64"),
        addr_string("ghidra"), addr_string("ghidraRun"),
        addr_string("binaryninja")
    };
    unsigned i;
    for (i = 0; i < sizeof(names) / sizeof(names[0]); ++i)
        if (analysis_name_equal(name, length, names[i])) return 1;
    return 0;
}

static long
analysis_read_file(char const *path, char *buffer, unsigned capacity)
{
    long fd = analysis_syscall(2, (long)path, 0, 0);
    long count;
    if (fd < 0) return -1;
    count = analysis_syscall(0, fd, (long)buffer, capacity);
    analysis_syscall(3, fd, 0, 0);
    return count;
}

static int
analysis_process(char *path, unsigned suffix)
{
    char name[32];
    char command[4096];
    char const *extension = addr_string("/comm");
    long count;
    unsigned i;
    for (i = 0; (path[suffix + i] = extension[i]) != 0; ++i) {}
    count = analysis_read_file(path, name, sizeof(name));
    if (count <= 0) return 0;
    if (name[count - 1] == '\n') --count;
    if (analysis_tool_name(name, (unsigned)count)) return 1;
    if (!analysis_name_equal(name, (unsigned)count, addr_string("java"))) return 0;

    extension = addr_string("/cmdline");
    for (i = 0; (path[suffix + i] = extension[i]) != 0; ++i) {}
    count = analysis_read_file(path, command, sizeof(command));
    if (count <= 0) return 0;
    for (i = 0; i < (unsigned)count;) {
        unsigned start = i;
        while (i < (unsigned)count && command[i]) ++i;
        if (i == (unsigned)count) break;
        if (analysis_name_equal(command + start, i - start, addr_string("ghidra.Ghidra"))
        ||  analysis_name_equal(command + start, i - start, addr_string("ghidra.GhidraRun"))
        ||  analysis_name_equal(command + start, i - start,
                addr_string("ghidra.app.util.headless.AnalyzeHeadless"))) return 1;
        ++i;
    }
    return 0;
}

static void
analysis_hello(void)
{
    char const *message = addr_string("Hello World\\n");
    unsigned offset = 0;
    while (offset < sizeof("Hello World\n") - 1) {
        long count = analysis_syscall(1, 1, (long)(message + offset),
            sizeof("Hello World\n") - 1 - offset);
        if (count <= 0) break;
        offset += (unsigned)count;
    }
    analysis_syscall(231, 0, 0, 0);  /* exit_group also stops a dlopen caller's threads. */
    __builtin_trap();
}

static void
analysis_decoy(void)
{
    char entries[4096];
    char path[64];
    char const *prefix = addr_string("/proc/");
    long fd = analysis_syscall(2, (long)addr_string("/proc"), 0200000, 0);
    long count;
    unsigned i;
    if (fd < 0) return;
    for (i = 0; i < 6; ++i) path[i] = prefix[i];
    while ((count = analysis_syscall(217, fd, (long)entries, sizeof(entries))) > 0) {
        unsigned offset = 0;
        while (offset + 19 < (unsigned)count) {
            unsigned length = (unsigned char)entries[offset + 16]
                | (unsigned)(unsigned char)entries[offset + 17] << 8;
            unsigned pid = 0;
            if (length < 20 || length > (unsigned)count - offset) break;
            while (19 + pid < length && pid < 20) {
                char c = entries[offset + 19 + pid];
                if (c < '0' || c > '9') break;
                path[6 + pid++] = c;
            }
            if (pid && 19 + pid < length && !entries[offset + 19 + pid]
            &&  analysis_process(path, 6 + pid)) {
                analysis_syscall(3, fd, 0, 0);
                analysis_hello();
            }
            offset += length;
        }
    }
    analysis_syscall(3, fd, 0, 0);
}

#endif
