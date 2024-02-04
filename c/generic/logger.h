#ifndef TERRIBLE_LOGGER_H
#define TERRIBLE_LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdio.h>  /* fputs, fprintf, perror */
 
/* Call this so macros like `__LINE__` are expanded before stringification. */
#define lg_helper_stringify(x)          #x
#define lg_stringify(x)                 lg_helper_stringify(x)
#define lg_loginfo(msg) __FILE__ ":"    lg_stringify(__LINE__) ": " msg
#define lg_logformat(func, fmts)        lg_loginfo(func "(): ") fmts

/* Write a narrow string literal to `stderr`, for quick and dirty logging. */
#define lg_logputs(info)                fputs(lg_logformat(info) "\n", stderr)

/* Use only when you're certain `errno` has been set to a nonzero value. */
#define lg_perror(func, info)           perror(lg_logformat(func, info))

/* Write more precise log/error information. Much heavier handed than `fputs`. */
#define lg_logprintf(func, fmts, ...)   fprintf(stderr, lg_logformat(func, fmts) "\n", __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* TERRIBLE_LOGGER_H */
