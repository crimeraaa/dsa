#include <stdio.h>
#include <string.h>

#include "allocator.h"
#include "strings.h"
#include "intern.h"

typedef struct {
    String *strings;
    int len;
} Args;

#define expand  string_expand

#define printfln(fmt, ...)  printf(fmt "\n", __VA_ARGS__)
#define println(msg)        printfln("%s", msg)

static void
run_interactive(Intern *intern)
{
    char buf[BUFSIZ];
    for (;;) {
        fputs(">>> ", stdout);
        if (!fgets(buf, cast(int)sizeof(buf), stdin)) {
            fputc('\n', stdout);
            break;
        }
        int len = cast(int)strcspn(buf, "\r\n");
        buf[len] = '\0';
        String key = {buf, len};
        String interned = intern_get(intern, key);
        printfln(STRING_QFMTSPEC " @ %p", expand(interned), cast(void *)interned.data);
    }
}

int
main(int argc, char *argv[])
{
    unused(argc);
    unused(argv);
    Intern intern = intern_make(HEAP_ALLOCATOR);
    run_interactive(&intern);
    intern_destroy(&intern);
    return 0;
}
