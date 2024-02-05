#ifndef CUSTOM_IO_INPUT_H
#define CUSTOM_IO_INPUT_H

#ifdef __cplusplus
extern "C" {
#else /* __cplusplus not defined. */
#ifndef nullptr
#define nullptr NULL 
#endif /* nullptr */
#endif /* __cplusplus */
    
#include <stdarg.h> /* va_list, vfprintf */
#include <stdio.h>  /* FILE, size_t, stdin, stdout, stderr, fprintf, perror */
#include <stdlib.h> /* malloc, calloc, realloc, free */
#include <string.h> /* strcspn */

#if defined(__GNUC__) 
/**
 * This is a GNU extension for compile-time checking of `printf` arguments.
 * See: https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html
 */
#define check_format(fmt, arg) \
    __attribute__((format(printf, fmt, arg)))
#elif defined(__clang__) 
/** 
 * Clang supports GNU's format attribute, although we surround them with
 * double underscores on both sides.
 */
#define check_format(fmt, arg) \
    __attribute__((__format__(__printf__, fmt, arg)))
#else /* __GNUC__ nor __clang__ defined. */
/* MSVC has no simple equivalent so leave empty. */
#define check_format(fmt, arg)
#endif /* __GNUC__ || __clang__ */
    
#define stringify_(expanded)    #expanded
#define stringify(toexpand)     stringify_(toexpand)
#define loginfo()               __FILE__ ":" stringify(__LINE__) ":"
#define logformat(fmts, ...)    loginfo() "%s: " fmts "\n", __func__, __VA_ARGS__
#define logperror(func, info)   perror(loginfo() func ": " info)
#define logputs(info)           fprintf(stderr, logformat("%s", info))
#define logprintf(fmts, ...)    fprintf(stderr, logformat(fmts, __VA_ARGS__));

/**
 * @brief   Bounded string length. Sort of an implementation of POSIX `strnlen`.
 *          Tries to find a pointer to first nul character in `subject`.
 *          See: https://stackoverflow.com/a/66346831 
 *
 * @return  Nul-terminated string length or fallback to `maximum` if no nul.
 */
size_t bd_strlen(const char *subject, size_t maximum);

/**
 * @brief   Actually handles reading user input and writing to a heap-allocated
 *          `char` buffer. This is called by `get_string`.
 */
char *readline(FILE *stream);

/**
 * @brief   An attempt to reimplement CS50's `get_string` using `fgets` rather
 *          than `fgetc`. I'm not sure how much I like this approach now...
 */
char *get_string(const char *fmts, ...) check_format(1, 2);

#ifdef __cplusplus
}
#endif /* extern "C" (1) */

#endif /* CUSTOM_IO_INPUT_H */
