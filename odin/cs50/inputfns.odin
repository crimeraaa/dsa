package cs50

import "core:bufio"
import "core:fmt"
import "core:io"
import "core:os"
import "core:strconv"

@private
Stdin :: struct {
    buffer: [256]byte,
    handle: os.Handle,
    stream: io.Stream,
    reader: bufio.Reader,
}

@(private)
stdin: Stdin

@(init)
init_internals :: proc() {
    stdin.handle = os.stdin
    stdin.stream = os.stream_from_handle(stdin.handle)
    bufio.reader_init_with_buf(&stdin.reader, stdin.stream, stdin.buffer[:])
}

/*
#### Brief:
Prompts the user to enter a line of text, terminated by a newline, to the 
standard input stream (a.k.a your terminal!)

#### Params:
`format` &mdash; C-style format string or string literal.

`args`   &mdash; Arguments to the format string, if any.

#### Returns:
Said line of text, likely encoded as UTF-8 since that's Odin's default.

#### Notes:
Although the string is heap-allocated, the package has takes care of its own 
memory via `cs50.stdin` and `cs50.stored`. 

If you want to manage the memory yourself you'll have to use `strings.clone`.
 */
get_string :: proc(format: string, args: ..any) -> string {
    fmt.printf(format, ..args)
    return readline(&stdin.reader)
}

/* 
#### Brief:
Wrapper around `cs50.get_string` which reinterprets the input as an `int`.

#### Returns:
The result of `strconv.parse_int`. Will loop until it receives a valid `int`.

#### Notes:
See `cs50.get_string` for more detailed information.
 */
get_int :: proc(format: string, args: ..any) -> int {
    for {
        input := get_string(format, ..args)
        value, ok := strconv.parse_int(input)
        if ok do return value
    }
}

/* 
#### Brief:
Wrapper around `cs50.get_string` which reinterprets the input as a `f64`.

#### Returns:
The result of `strconv.parse_f64`. We'll loop until a valid `f64` is received.

#### Notes:
See `cs50.get_string` for more detailed information.
 */
get_float :: proc(format: string, args: ..any) -> f64 {
    for {
        input := get_string(format, ..args)
        value, ok := strconv.parse_f64(input)
        if ok do return value
    }
}
