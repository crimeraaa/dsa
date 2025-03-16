#pragma once

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

#ifndef cast
#define cast(Type)  (Type)
#endif // cast

#ifndef unused
#define unused(expression)  cast(void)(expression)
#endif // unused

#ifndef count_of
#define count_of(literal) (sizeof(literal) / sizeof((literal)[0]))
#endif // count_of

#define print(msg)          printf("%s", msg)
#define printfln(fmt, ...)  printf(fmt "\n", __VA_ARGS__)
#define println(msg)        printfln("%s", msg)

#define fprint(stream, msg)         fprintf(stream, "%s", msg)
#define fprintfln(stream, fmt, ...) fprintf(stream, fmt "\n", __VA_ARGS__)
#define fprintln(stream, msg)       fprintfln(stream, "%s", msg)

#define eprintf(fmt, ...)   fprintf(stderr, fmt, __VA_ARGS__)
#define eprint(msg)         eprintf("%s", msg)
#define eprintfln(fmt, ...) eprintf(fmt "\n", __VA_ARGS__)
#define eprintln(msg)       eprintfln("%s", msg)
