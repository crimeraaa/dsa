#include <string.h>

#define ALLOCATOR_IMPLEMENTATION
#include "allocator.h"

#define ASCII_IMPLEMENTATION
#define STRINGS_IMPLEMENTATION
#define STRINGS_BUILDER_IMPLEMENTATION
#include "strings.h"

#define INTERN_IMPLEMENTATION
#include "intern.h"

#include "types/types.h"
#include "types/lexer.h"

static void
run_interactive(Type_Table *table)
{
    char buf[512];
    for (;;) {
        fputs(">>> ", stdout);
        if (!fgets(buf, cast(int)sizeof(buf), stdin)) {
            fputc('\n', stdout);
            break;
        }

        size_t len = strcspn(buf, "\r\n");
        buf[len] = '\0';
        printfln("\'%s\'", buf);
        if (!type_parse_string(table, buf, len)) {
            printfln("Invalid type '%s'.", buf);
        }
    }
}

int
main(void)
{
    Type_Table table = type_table_make(GLOBAL_PANIC_ALLOCATOR);
    run_interactive(&table);
    type_table_destroy(&table);
    return 0;
}
