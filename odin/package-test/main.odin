package test

import "core:fmt" 
import "../cs50"

main :: proc() {
    defer cs50.deinit()
    fv := cs50.get_file_view("./sample.txt")
    for line, index in fv.lines {
        fmt.println("Ln", index + 1, ":", line)
    }
    name := cs50.get_string("Where are we? ")
    year := cs50.get_int("What year is it on %s? ", name)
    temp := cs50.get_float("What's the temperature on %s? ", name)
    fmt.println(name, year, temp)
}
