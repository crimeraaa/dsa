package custom_io

import "core:fmt"
import "core:os"
import "core:math/rand"

FILE_NAMES: []string = {"./sample.txt", "./nope.txt"}

main :: proc() {
    defer delete(os.args)
    fmt.println("-- begin test --")
    file_name  := rand.choice(FILE_NAMES[:]) if len(os.args) != 2 else os.args[1]
    file_lines := readfile(file_name)
    defer delete(file_lines)
    fmt.println(file_lines)
    fmt.println("-- end test --")

    letter := get_rune("Enter a letter: ")
    fmt.println("You entered:", letter)
    // try_gs_dynamic()
    try_gs_fixed()
}

try_gs_fixed :: proc() {
    buf1: [256]byte
    name := fixed_get_string(buf1[:], "What planet is this? ")

    buf2: [256]byte
    year := fixed_get_string(buf2[:], "What year is it on %s? ", name)

    buf3: [256]byte
    temp := fixed_get_string(buf3[:], "How how is it on %s at year %s? ", name, year)

    print_test_inputs(name, year, temp)
}

try_gs_dynamic :: proc() {
    name := get_string("What planet is this? ")
    defer delete(name)

    year := get_string("What year is it on %s? ", name)
    defer delete(year)

    temp := get_string("How hot is it on %s at year %s? ", name, year)
    defer delete(temp)

    print_test_inputs(name, year, temp)
}

print_test_inputs :: proc(name: string, year: string, temp: string) {
    fmt.println(name, len(name))
    fmt.println(year, len(year))
    fmt.println(temp, len(temp))
}
