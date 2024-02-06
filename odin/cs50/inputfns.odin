package cs50

import "core:fmt"     // .print, .println, .printf
import "core:os"      // .stdin, .stream_from_handle,

/*
#### Brief:
Use `..` to "unpack" varargs, otherwise you'll print the struct!

#### Links:
<https://odin-lang.org/docs/overview/#ternary-operators>
*/
get_string :: proc(format: string, args: ..any) -> string {
    // Can't use ternary operators as they're for assignment only.
    fmt.printf(format, ..args)        
    return readline(os.stream_from_handle(os.stdin))
}
