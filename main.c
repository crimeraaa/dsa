#include <stdio.h>

#include "allocator.h"
#include "strings.h"

typedef struct {
    String *strings;
    int len;
} Args;

#define expand  string_expand

#define printfln(fmt, ...)  printf(fmt "\n", __VA_ARGS__)
#define println(msg)        printfln("%s", msg)

int
main(int argc, char *argv[])
{
    Args args;
    args.strings = mem_make(String, cast(size_t)argc, HEAP_ALLOCATOR);
    args.len     = argc;
    for (int i = 0; i < args.len; i++) {
        args.strings[i] = string_from_cstring(argv[i]);
    }
    
    for (int i = 0; i < args.len; i++) {
        String arg = args.strings[i];
        printf("[%i]: " STRING_QFMTSPEC " => ", i, arg.len, arg.data);
        for (String state = arg, it; string_split_iterator(&state, ' ', &it);) {
            printf(STRING_QFMTSPEC ", ", expand(it));
        }
        printf("\n");
        // printf("space @ %i\n", string_index_substring(arg, string_from_literal(" ")));
    }
    // args.strings = NULL;
    mem_delete(args.strings, cast(size_t)args.len, HEAP_ALLOCATOR);
    return 0;
}
