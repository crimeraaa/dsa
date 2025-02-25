#include <stdio.h>
#include <string.h>

#include "allocator.h"
#include "strings.h"
#include "intern.h"

#define expand  string_expand

#define printfln(fmt, ...)  printf(fmt "\n", __VA_ARGS__)
#define println(msg)        printfln("%s", msg)

#define eprintf(fmt, ...)   fprintf(stderr, fmt, __VA_ARGS__)
#define eprint(msg)         eprintf("%s", msg)
#define eprintfln(fmt, ...) eprintf(fmt "\n", __VA_ARGS__)
#define eprintln(msg)       eprintfln("%s", msg)

static void
run_interactive(Intern *intern)
{
    char buf[256];
    for (;;) {
        fputs(">>> ", stdout);
        if (!fgets(buf, cast(int)sizeof(buf), stdin)) {
            fputc('\n', stdout);
            break;
        }
        String key      = {buf, strcspn(buf, "\r\n")};
        String interned = intern_get(intern, key);
        printfln(STRING_QFMTSPEC " @ %p", expand(interned), cast(void *)interned.data);

        for (String state = interned, it; string_split_char_iterator(&state, &it, ' '); ) {
            printfln("\t- " STRING_QFMTSPEC, expand(it));
        }
    }

    intern_print(intern, stdout);
}

static void
_intern_input(Intern *intern, String input)
{
    for (String state = input, line; string_split_lines_iterator(&state, &line);) {
        // We never read `line` within `string_split_lines_iterator`, so we
        // should be able to mutate it however we want without invalidating the
        // outer loop.
        for (String word; string_split_char_iterator(&line, &word, ' ');) {
            intern_get(intern, word);
        }
    }
}

static void
_intern_file(Intern *intern, const char *file_name)
{
    FILE *file_ptr = fopen(file_name, "rb");
    if (file_ptr == NULL) {
        eprintfln("Failed to open file '%s'.", file_name);
        return;
    }

    fseek(file_ptr, 0, SEEK_END);
    long _file_size = ftell(file_ptr);
    if (_file_size == -1) {
        eprintfln("Failed to get size of file '%s'.", file_name);
        goto cleanup_file;
    }

    size_t file_size = cast(size_t)_file_size;
    rewind(file_ptr);

    char *buf = mem_make(char, file_size + 1, intern->allocator);
    if (buf == NULL) {
        eprintfln("Failed to allocate %zu bytes.", file_size);
        goto cleanup_buf;
    }
    if (fread(buf, sizeof(buf[0]), file_size, file_ptr) != file_size) {
        eprintfln("Failed to read ilfe '%s'.", file_name);
        goto cleanup_buf;
    }
    buf[file_size] = '\0';

    String input = {buf, file_size};
    _intern_input(intern, input);

cleanup_buf:
    mem_delete(buf, file_size + 1, intern->allocator);
cleanup_file:
    fclose(file_ptr);
}

int
main(int argc, char *argv[])
{
    Intern intern = intern_make(HEAP_ALLOCATOR);
    if (argc == 1) {
        run_interactive(&intern);
    } else if (argc >= 2) {
        for (int i = 1; i < argc; ++i) {
            printf("Interning: '%s'\n", argv[i]);
            _intern_file(&intern, argv[i]);
        }
        FILE *output = fopen("output.txt", "w+");
        intern_print(&intern, output);
        // if (argc == 2)
        //     intern_print(&intern, stdout);
        fclose(output);
    }
    intern_destroy(&intern);
    return 0;
}
