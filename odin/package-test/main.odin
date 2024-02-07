package test

import "core:fmt" 
import "../cs50"

main :: proc() {
    defer cs50.clear_allocations()
    file := cs50.readfile("./sample.txt")
    for line, index in file {
        fmt.println("Ln", index + 1, ":", line)
    }
    name := cs50.get_string("Where are we? ")
    year := cs50.get_int("What year is it on %s? ", name)
    temp := cs50.get_float("What's the temperature on %s? ", name)
    fmt.println(name, year, temp)
}
