/// local
#define ARENA_IMPLEMENTATION
#define ALLOCATOR_IMPLEMENTATION
#define ASCII_IMPLEMENTATION
#define STRINGS_IMPLEMENTATION
#define STRINGS_BUILDER_IMPLEMENTATION
#define INTERN_IMPLEMENTATION
#define TYPES_IMPLEMENTATION

#include "mem/allocator.h"
#include "mem/arena.h"
#include "strings.h"
#include "intern.h"

#include "types/types.h"
#include "types/lexer.h"

/// standard
#include <string.h>

static void
run_interactive(Arena *arena)
{
    Allocator temp_allocator = arena_allocator(arena);
    char buf[256];
    for (;;) {
        fputs(">>> ", stdout);
        if (!fgets(buf, cast(int)sizeof(buf), stdin)) {
            fputc('\n', stdout);
            break;
        }

        C_Lexer lexer = c_lexer_make(buf, strcspn(buf, "\r\n"));
        println("=== TOKENS ===");
        for (;;) {
            C_Token token = c_lexer_scan(&lexer);
            if (!token.type) {
                printfln("[ERROR]: Invalid token " STRING_QFMTSPEC ".", string_fmtarg(token.word));
                break;
            } else if (token.type == C_TokenType_Eof) {
                break;
            }
            printfln(
                "%s: " STRING_QFMTSPEC,
                c_token_strings[token.type].data,
                string_fmtarg(token.word));
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
    Arena           arena;
    Allocator_Error error = arena_init(&arena);
    if (error)
        return 1;

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

    run_interactive(&arena);
    arena_destroy(&arena);
    return 0;
}
