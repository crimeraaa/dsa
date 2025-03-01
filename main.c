#include <string.h>

#include "allocator.h"
#include "strings.h"
#include "intern.h"
#include "types.h"

static void
run_interactive(void)
{
    char buf[512];
    for (;;) {
        fputs(">>> ", stdout);
        if (!fgets(buf, cast(int)sizeof(buf), stdin)) {
            fputc('\n', stdout);
            break;
        }

        String name = {.data = buf, .len = strcspn(buf, "\r\n")};
        printfln(STRING_QFMTSPEC, STRING_FMTARG(name));
        if (!type_parse_string(name)) {
            printfln("Invalid type " STRING_QFMTSPEC ".",
                STRING_FMTARG(name));
        }
    }
}

int
main(void)
{
    run_interactive();
    return 0;
}
