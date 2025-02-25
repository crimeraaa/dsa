#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#ifndef cast
#define cast(Type)  (Type)
#endif // cast

#ifndef unused
#define unused(expression)  cast(void)(expression)
#endif // unused

#ifndef count_of
#define count_of(literal) (sizeof(literal) / sizeof((literal)[0]))
#endif // counf_of

#define printfln(fmt, ...)  printf(fmt "\n", __VA_ARGS__)
#define println(msg)        printfln("%s", msg)

#define eprintf(fmt, ...)   fprintf(stderr, fmt, __VA_ARGS__)
#define eprint(msg)         eprintf("%s", msg)
#define eprintfln(fmt, ...) eprintf(fmt "\n", __VA_ARGS__)
#define eprintln(msg)       eprintfln("%s", msg)

#endif // COMMON_H
