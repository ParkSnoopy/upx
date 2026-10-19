#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#define FIXTURE_EXPORT __declspec(dllexport)
#define FIXTURE_TLS    __declspec(thread)
#else
#include <dlfcn.h>
#define FIXTURE_EXPORT
#define FIXTURE_TLS _Thread_local
#endif

static unsigned initialized;
static unsigned *volatile relocated = &initialized;
static FIXTURE_TLS unsigned thread_value = 7;
static const unsigned char payload[32768] = "Opaque runtime fixture";

#if defined(UPX_OPAQUE_SHARED_FIXTURE)
#if defined(_WIN32)
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
    (void) instance;
    (void) reserved;
    if (reason == DLL_PROCESS_ATTACH)
        initialized = 1;
    return TRUE;
}
#else
__attribute__((constructor)) static void fixture_init(void) { initialized = 1; }
#endif
#endif

FIXTURE_EXPORT unsigned opaque_fixture(unsigned index);
FIXTURE_EXPORT unsigned opaque_fixture(unsigned index) {
    const volatile unsigned char *bytes = payload;
    if (*relocated != 1 || index > 1 || thread_value != 7 + index || bytes[0] != 'O' ||
        bytes[sizeof(payload) - 1] != 0)
        return 0;
    ++thread_value;
    return 42;
}

#if !defined(UPX_OPAQUE_SHARED_FIXTURE)
static const char *fixture_path(const char *requested) {
    static const char *const paths[] = {UPX_OPAQUE_TARGETS};
    for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); ++i)
        if (strcmp(requested, paths[i]) == 0)
            return paths[i];
    return NULL;
}

static int load_fixture(const char *path, int lazy) {
    for (unsigned i = 0; i < 2; ++i) {
#if defined(_WIN32)
        (void) lazy;
        HMODULE library = LoadLibraryA(path);
        if (!library) {
            fprintf(stderr, "LoadLibrary failed: %lu\n", GetLastError());
            return 1;
        }
        unsigned (*fixture)(unsigned) =
            (unsigned (*)(unsigned)) GetProcAddress(library, "opaque_fixture");
        int failed = !fixture || fixture(0) != 42 || fixture(1) != 42;
        if (!FreeLibrary(library))
            failed = 1;
#else
        void *library = dlopen(path, (lazy ? RTLD_LAZY : RTLD_NOW) | RTLD_LOCAL);
        if (!library) {
            fprintf(stderr, "%s\n", dlerror());
            return 1;
        }
        unsigned (*fixture)(unsigned) = (unsigned (*)(unsigned)) dlsym(library, "opaque_fixture");
        int failed = !fixture || fixture(0) != 42 || fixture(1) != 42;
        if (dlclose(library) != 0)
            failed = 1;
#endif
        if (failed)
            return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    initialized = 1;
    if (opaque_fixture(0) != 42 || opaque_fixture(1) != 42)
        return 1;
    if (argc == 3 && (strcmp(argv[1], "--load") == 0 || strcmp(argv[1], "--load-lazy") == 0)) {
        const char *path = fixture_path(argv[2]);
        if (!path)
            return 2;
        if (load_fixture(path, strcmp(argv[1], "--load-lazy") == 0))
            return 1;
    } else if (argc != 1) {
        return 2;
    }
    puts("Opaque runtime fixture passed");
    return 0;
}
#endif
