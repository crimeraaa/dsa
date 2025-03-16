/// local
#define DSA_IMPLEMENTATION

#include "mem/allocator.h"
#include "mem/arena.h"
#include "strings.h"
#include "intern.h"

#include "types/types.h"
#include "types/lexer.h"
#include "types/parser.h"

/// standard
#include <string.h>

static Arena arena;

static void
run_interactive(Arena *arena, CType_Table *table)
{
    Allocator temp_allocator = arena_allocator(arena);
    char buf[256];
    for (;;) {
        fputs(">>> ", stdout);
        if (!fgets(buf, cast(int)sizeof(buf), stdin)) {
            fputc('\n', stdout);
            break;
        }

        CLexer  lexer  = clexer_make(buf, strcspn(buf, "\r\n"));
        CParser parser = {
            .type       = ctype_basic_types[CType_BasicKind_Invalid],
            .qualifiers = 0,
            .flags      = 0,
        };
        println("=== TOKENS ===");
        if (cparser_parse(&parser, &lexer)) {
            String_Builder builder = string_builder_make(temp_allocator);
            const char *canonical_name = cparser_canonicalize(&parser, &builder);
            printfln("Expr: %s = '%s'", ctype_kind_strings[parser.type.kind].data, canonical_name);

            // WARNING: Assumes a lot about `parser.type`!
            const CType_Info *info = ctype_table_get_basic_qual(table, parser.type.basic.kind, parser.qualifiers);
            if (info) {
                printfln("Found info @ %p", cast(void *)info);
            } else {
                println("First time seeing info. Adding...");
                info = ctype_table_add_basic_qual(table, parser.type.basic.kind, parser.qualifiers);
                if (info)
                    printfln("Found info @ %p", cast(void *)info);
                else
                    println("Could not add info!");
            }
        }
        println("==============\n");

        size_t total;
        size_t used  = arena_get_usage(arena, &total);
        printfln(
            "=== ARENA INFO ===\n"
            "Begin: %p\n"
            "End:   %p\n"
            "Usage: %zu bytes (out of %zu)\n"
            "==================\n",
            cast(void *)arena->begin,
            cast(void *)arena->end,
            used, total);
        mem_free_all(temp_allocator);
    }
}

int
main(void)
{
    Allocator_Error error = arena_init(&arena);
    if (error)
        return 1;

    CType_Table table;
    error = ctype_table_init(&table, GLOBAL_PANIC_ALLOCATOR);
    if (error)
        return 1;

    run_interactive(&arena, &table);
    arena_destroy(&arena);
    ctype_table_destroy(&table);
    return 0;
}
