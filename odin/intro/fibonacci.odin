package hello

import "core:fmt"

// Odin resolves all function calls, so we don't need to declare them in order.
print_fibonacci :: proc(reps: int) {
    /*  Unlike in C, you can omit semicolons for empty expressions.
        e.g. for {} is an infinite loop!
        `for i := 0; i < reps; i += 1` can be rewritten succinctly as: */
    for i in 0..<reps {
        fmt.printf("fib[%i] = %i\n", i, get_fibonacci(i))
    }
}

/* 
Get the `n`'th number in the fibonacci sequence
Assumes `fib[0] = 0` and `fib[] = 1`.

Also, switch statements DON'T fallthrough by default!
If you'd need that, use the `fallthrough` keyword.
 */
get_fibonacci :: proc(n: int) -> int {
    // Recurse base cases to avoid infinite recursion
    switch {
        case n <= 0: return 0
        case n == 1: return 1
    }
    // e.g. n = 2, so fibonacci is (2 - 1) + (2 - 2) = 1 + 0 = 1.
    return get_fibonacci(n - 1) + get_fibonacci(n - 2)
}

/* 
Old, C-style implementation for reference.
```odin
print_fibonacci :: proc(reps: int) {
    alpha: int = 0
    bravo: int = 1
    charlie: int = alpha + bravo
    for i := 0; i < reps; i += 1 {
        fmt.printf("fib @ %i = %i\n", i, charlie)
        alpha, bravo = bravo, charlie
        charlie = alpha + bravo
    }
}
```
*/
