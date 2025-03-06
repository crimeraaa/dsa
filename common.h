#pragma once

/**=============================================================================
 * We need to define `_DEFAULT_SOURCE` here because of our header-only nature.
 *
 *      This also means that for `arena.h` to work, you MUST include *this* file
 *      (or include a file that includes this one) BEFORE any standard headers.
 * 
 *      `_DEFAULT_SOURCE` is used so that `sys/mman.h` will define `MAP_ANONYMOUS`,
 *      There is probably a better way of enabling it...
 *=============================================================================*/
#ifdef __unix__

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif // _DEFAULT_SOURCE

#endif // __unix__

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
