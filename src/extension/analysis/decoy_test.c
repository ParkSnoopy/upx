#if defined(UPX_EXTENSION_SHARED_FIXTURE)

int extension_fixture(void);
int extension_fixture(void) { return 42; }

#else

#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>

static char const *fixture_path(char const *requested)
{
    static char const *const paths[] = { UPX_EXTENSION_TARGETS };
    for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); ++i)
        if (strcmp(requested, paths[i]) == 0) return paths[i];
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--fixture") == 0) {
        puts("Original program");
        return 0;
    }
    if (argc == 3 && strcmp(argv[1], "--load") == 0) {
        char const *path = fixture_path(argv[2]);
        if (!path) return 2;
        void *library = dlopen(path, RTLD_NOW | RTLD_LOCAL);
        int (*fixture)(void);
        if (!library) {
            fprintf(stderr, "%s\n", dlerror());
            return 1;
        }
        fixture = (int (*)(void))dlsym(library, "extension_fixture");
        if (!fixture || fixture() != 42) return 1;
        if (dlclose(library) != 0) return 1;
        puts("Original program");
        return 0;
    }
    if (argc != 6 && argc != 7) return 2;
    char const *target = fixture_path(argv[1]);
    char const *packed = fixture_path(argv[3]);
    if (!target || !packed) return 2;

    /* Name the waiting parent, not the target, to exercise a separate process. */
    if (prctl(PR_SET_NAME, argv[4], 0, 0, 0) != 0) return 1;
    int output[2];
    if (pipe(output) != 0) return 1;
    pid_t child = fork();
    if (child < 0) {
        close(output[0]);
        close(output[1]);
        return 1;
    }
    if (child == 0) {
        close(output[0]);
        if (dup2(output[1], STDOUT_FILENO) < 0) _exit(126);
        close(output[1]);
        if (strcmp(argv[2], "main") == 0)
            execl(target, target, "--fixture", (char *)0);
        else
            execl(target, target, "--load", packed, (char *)0);
        _exit(127);
    }
    close(output[1]);
    char actual[128];
    size_t used = 0;
    ssize_t count;
    while (used < sizeof(actual)) {
        count = read(output[0], actual + used, sizeof(actual) - used);
        if (count < 0 && errno == EINTR) continue;
        if (count <= 0) break;
        used += (size_t)count;
    }
    close(output[0]);
    int status;
    pid_t result;
    do result = waitpid(child, &status, 0); while (result < 0 && errno == EINTR);
    if (strcmp(argv[5], "decoy") == 0) {
        if (result != child || !WIFSIGNALED(status) || WTERMSIG(status) != SIGSEGV
        || used != 0) {
            fprintf(stderr, "Expected SIGSEGV, status %#x, output: %.*s\n",
                status, (int)used, actual);
            return 1;
        }
    } else {
        char const *expected = "Original program\n";
        if (result != child || !WIFEXITED(status) || WEXITSTATUS(status) != 0
        || used != strlen(expected) || memcmp(actual, expected, used) != 0) {
            fprintf(stderr, "Expected: %sActual: %.*s\n", expected, (int)used, actual);
            return 1;
        }
    }
    return 0;
}

#endif
