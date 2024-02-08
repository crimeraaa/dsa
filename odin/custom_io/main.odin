package custom_io

import "core:fmt"

main :: proc() {
    fmt.println("-- begin test --")
    file_lines := readfile("./nope.txt")
    defer delete(file_lines)
    fmt.println(file_lines)
    fmt.println("-- end test --")
    // letter := get_rune("Enter a letter: ")
    // fmt.println("You entered:", letter)

    // name := get_string_dynamic("What planet is this? ")
    // year := get_string_dynamic("What year is it on %s? ", name)
    // temp := get_string_dynamic("How hot is it on %s at year %s? ", name, year)
    // defer delete(name)
    // defer delete(year)
    // defer delete(temp)

    // fmt.println(name, len(name))
    // fmt.println(year, len(year))
    // fmt.println(temp, len(temp))
}
