#pragma once

/**
 * BRIEF:   Inspired from the CS50 library for C.
 * 
 * SOURCE:  https://github.com/cs50/libcs50/blob/main/src/cs50.c
*/
#if defined(__GNUC__) /* __attribute__((contructor)) is a GCC extension. */
/* Write the function body, braces included, after calling this. */
#define program_init(name) \
    static void name(void) __attribute__((constructor)); \
    static void name(void)
#elif defined(_MSC_VER)
#pragma section(".CRT$XCU", read)
/**
 * Taken from Joe Lowe's answer on Stack Overflow. This is roughly the 
 * Microsoft Visual C/C++ equivalent of GCC's constructor attribute.
 * 
 * We defined a void (void) function to be called before program/library entry.
 * We then place a function pointer to this function in the initializer section.
 * (This is the ".CRT$XCU") business.
 * 
 * This is the same thing the compiler does for static C++ object constructors.
 * - https://stackoverflow.com/a/2390626
 */
#define place_fnptr_at_initializer(name, prefix) \
    static void name(void);
    __declspec(allocate(".CRT$XCU")) void (*name##_)(void) = name; \
    __pragma(comment(linker, "/include:" prefix #name "_")) \
    static void name(void)
#ifdef _WIN64
    #define program_init(name) place_at_initializer(name, "")
#else /* _WIN64 not defined so linker needs '_' prefix */
    #define program_init(name) place_at_initializer(name, "_")
#endif /* _WIN64 */
#else /* __GNUC__ nor _MSC_VER defined */
#error Your compiler does not support constructors. If you are confident to push through anyway, comment this error out.
#endif /* __GNUC__, _MSC_VER */
