#include <cstdarg>
#include <cstdio>
#include <string>

#ifdef __GNUC__
/* This will only work with printf family. wprintf family is not supported. */
#define CHECK_FORMAT(fmts, args) __attribute__((format (printf, fmts, args)))
#else /* __GNUC__ not defined: fall back to empty macro to avoid error */
#define CHECK_FORMAT(fmts, args)
#endif /* __GNUC__ */

/**
 * @brief   Create a C-style formatted string and send it to a container.
 *
 * @tparam  ...Args     Variadic arguments to be passed to `snprintf`.
 * @param   fmts        C-string literal or format string to deduce arguments.
 * @param   ...args     Arguments to `fmts`, if any were provided.
 * @return 
 * 
 * @note    This will cause a LOT of template function instantiations.
 *          That is, every signature you use is a different functon in memory.
 *          Have fun!
 */
template<typename ...Args>
std::string csformat(const char *fmts, Args ...args) CHECK_FORMAT(1, 2);

template<typename ...Args>
std::string csformat(const char *fmts, Args ...args)
{
    std::string writer;
    int n_length; // Keep track of return values of calls to `snprintf`.

    // Unpack template varargs this way
    n_length = snprintf(nullptr, 0, fmts, args...); 
    if (n_length < 0) {
        goto endfn;
    }
    writer.reserve(n_length + 1); // Extra capacity for nul termination
    writer.resize(n_length); // Affects `writer.length()` properly
    n_length = snprintf(writer.data(), writer.capacity(), fmts, args...);
    if (n_length < 0) {
        std::string none; // Will clears any allocated memory from `writer`
        writer.swap(none);
    }
endfn:
    return writer;    
}
