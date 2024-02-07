package cs50

import "core:fmt"
import "core:os"

main :: proc() {
    defer delete(os.args) // This is also allocated!!
    defer deinit()
    file_path := os.args[1] if len(os.args) > 1 else "./sample.txt"
    fv := get_file_view(file_path)
    for line, index in fv.lines {
        fmt.println("Ln", index + 1, ":", line)
    }
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
