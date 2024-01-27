#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdio.h>

#ifdef __GNUC__
#define check_format(fmts, args) __attribute__ ((format (printf, fmts, args)))
#else /* __GNUC__ not defined, so leave it as an empty macro. */
#define check_format(fmts, args)
#endif /* __GNUC__ */

/** 
 * @brief   My personal implementation of C-style printf. For fun!
 *
 * @param   stream      Where the output will go.
 * @param   fmts        Format string with 0/more format specifiers.
 * @param   ...         Arguments to the format string.
 *
 * @note    This is (should be, anyway) C compatible!
 */
void write_format(FILE *stream, const char *fmts, ...) check_format(2, 3);

#ifdef __cplusplus
}
#endif
