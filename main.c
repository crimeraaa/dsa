/// local
#define ARENA_IMPLEMENTATION
#define ALLOCATOR_IMPLEMENTATION
#define ASCII_IMPLEMENTATION
#define STRINGS_IMPLEMENTATION
#define STRINGS_BUILDER_IMPLEMENTATION
#define INTERN_IMPLEMENTATION

#include "mem/allocator.h"
#include "mem/arena.h"
#include "strings.h"
#include "intern.h"

#include "types/types.h"
#include "types/lexer.h"

/// standard
#include <string.h>

static void
run_interactive(Type_Table *table, Arena *arena)
{
    Allocator temp_allocator = arena_allocator(arena);
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
        if (!type_parse_string(table, buf, len, temp_allocator)) {
            printfln("Invalid type '%s'.", buf);
        }
        
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
    Arena           arena;
    Allocator_Error error = arena_init(&arena);
    if (error)
        return 1;

    Type_Table table = type_table_make(GLOBAL_PANIC_ALLOCATOR);

    // testing for alignment
    {
        char *ch = arena_rawalloc(&arena, sizeof(char), alignof(char));
        *ch = 'a';
        printfln("char *ch: %p; *ch = '%c'", cast(void *)ch, *ch);

        int *i  = arena_rawalloc(&arena, sizeof(int), alignof(int));
        *i = 23;
        printfln("int *i: %p; *i = %i", cast(void *)i, *i);

        // `p` points to a `void *`.
        void **p = arena_rawalloc(&arena, sizeof(void *), alignof(void *));
        *p = ch;
        printfln("void **p: %p; *p = %p\n", cast(void *)p, *p);

        arena_free_all(&arena);
    }

    run_interactive(&table, &arena);
    type_table_destroy(&table);
    arena_destroy(&arena);
    return 0;
}
