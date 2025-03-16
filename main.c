/// local
#define DSA_IMPLEMENTATION

#include "mem/allocator.h"
#include "mem/arena.h"
#include "strings.h"
#include "intern.h"

#include "types/types.h"

/// standard
#include <string.h>

static void
run_interactive(CType_Table *table)
{
    char buf[256];

    ctype_table_print(table);
    for (;;) {
        fputs(">>> ", stdout);
        if (!fgets(buf, cast(int)sizeof(buf), stdin)) {
            fputc('\n', stdout);
            break;
        }

        println("=== TOKENS ===");
        const CType_Info *info = ctype_get(table, buf, strcspn(buf, "\r\n"));
        if (info != NULL) {
            printfln("Expr : %s : '%s' (%p)",
                ctype_kind_strings[info->type->kind].data,
                info->name->data,
                cast(void *)info);
        }
        println("==============\n");

        size_t total;
        size_t used  = arena_get_usage(&_global_arena, &total);
        printfln(
            "=== ARENA INFO ===\n"
            "Begin: %p\n"
            "End:   %p\n"
            "Usage: %zu bytes (out of %zu)\n"
            "==================\n",
            cast(void *)_global_arena.begin,
            cast(void *)_global_arena.end,
            used, total);
        mem_free_all(global_temp_allocator);
    }
    ctype_table_print(table);
}

int
main(void)
{
    if (global_temp_allocator_init())
        return 1;

    Intern          intern = intern_make(global_panic_allocator);
    CType_Table     table;
    Allocator_Error error = ctype_table_init(&table, &intern, global_panic_allocator);
    if (error)
        return 1;

    run_interactive(&table);
    arena_destroy(&_global_arena);
    ctype_table_destroy(&table);
    intern_destroy(&intern);
    return 0;
}
