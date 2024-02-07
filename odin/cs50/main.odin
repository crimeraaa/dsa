package cs50

import "core:fmt"

main :: proc() {
    defer clear_allocations()
    // filelines := readfile("./sample.txt")
    // for line, index in filelines {
    //     fmt.println("Ln", index + 1, ":", line)
    // }
    planet := get_string("What planet is this? ")
    year   := get_int("What's the current year on %s? ", planet)
    temp   := get_float("What's the temperature on %s, (year %i)? ", planet, year)
    print_input("planet", planet)
    print_input("year", year)
    print_input("temp", temp)
}

// See here: https://odin-lang.org/docs/overview/#exported-names
@(private="file") 
print_input :: proc(ident: string, value: any) {
    fmt.println(ident, ":=", value)
}
