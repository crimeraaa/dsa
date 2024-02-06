// This declares a package called "hello". Odin is package-based!
// That means that it considers entire directories at a time.
package hello

/* The core library includes lots of useful utilities ala C's stdio.h. */
import "core:fmt"

// This is the entry point, similar to C, but only for this package.
main :: proc() {
    test_strings_and_literals()
    print_fibonacci(10)
    name := get_string("Enter your name: ")
    defer delete(name) // Was allocated by `get_string`!
    fmt.printf("Hi %s!\n", name)
}

// Allow us to store either a character literal (a `rune`) OR a UTF-8 string.
C_Str :: union {
    rune,
    string,
}

// Odin features some string/character handling siimliar to C.
test_strings_and_literals :: proc() {
    s: C_Str = "Hello there" // String literals are enclosed by double quotes.
    c: C_Str = 'c' // Character literals are enclosed by single quotes.
    r: C_Str = `C:\Program Files (x86)\` // Raw string literals (no escaped chars)
    print_string('s', s)
    print_string('c', c)
    print_string('r', r)
}

print_string :: proc(ident: rune, value: C_Str) {
    // Will be allocated to by `fmt.aprintf`.
    output: string 
    // We can query the type at runtime!
    // https://odin-lang.org/docs/overview/#type-switch-statement
    switch v in value {
        case string: output = fmt.aprintf("\"%s\", len = %i", v, len(v))
        case rune:   output = fmt.aprintf("\'%c\', len = %i", v, 1)
    }
    fmt.println("%s\n", output)
    // Frees backing memory of a value associate with our string
    // That was allocated by `context.allocator`.
    // See: https://odin-lang.org/docs/overview/#implicit-context-system
    delete(output)
}
