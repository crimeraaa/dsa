package tests

import "core:fmt"
import "shared:crim" 

/* 
Pass the following flag on the command line:

`-collection:shared=/home/crimeraaa/repos/dsa/odin/shared`

After the `build` command, e.g:

`odin build . -out:bin/main -collection:shared=/home/crimeraaa/repos/dsa/odin/shared`
*/
main :: proc() {
    name := crim.get_string("What planet is this? ")
    defer delete(name)
    
    year := crim.get_int("What year is it on %s? ", name)
    temp := crim.get_float("How hot is it on %s, at year %i? ", name, year)

    fmt.println(name, year, temp)
}
