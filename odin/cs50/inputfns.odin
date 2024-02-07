package cs50

import "core:bufio"
import "core:fmt"
import "core:io"
import "core:os"
import "core:strconv"
import "core:strings"
import "core:mem"

/* 
#### Brief
Intended to help reduce allocations by allocating memory for 1 string,
then using a slice (a view into a dynamic array) where each element is itself
a view into the main string.

#### Fields
`buffer` &mdash; The main allocated string, read from `os.read_entire_file`.

`lines`  &mdash; A slice of a dynamic array created by `cs50.readfile`.
 */
File_View :: struct {
    buffer: string,
    lines: []string,
}

@(private)
stdin: Wrap_IO

@(init)
init :: proc() {
    wrap_io_init(&stdin, os.stdin, os.O_RDONLY)
}

/* 
#### Brief:
Delete all the allocated strings and string arrays created by all of the
`cs50.get_*` functions, and also destroys `cs50.stdin.reader`.

#### Sample:
```odin
import "core:fmt"
import "cs50"

main :: proc() {
    defer cs50.deinit()
    name = cs50.get_string("Enter your name: ")
    fmt.printf("Hi %s!\n", name) 
}
```

#### Note:
`cs50.readline` and `cs50.readfile` do not append to the `cs50` internal arrays.
This is useful if you wish to manage the memory yourself.
*/
deinit :: proc() {
    defer wrap_io_deinit(&stdin)
    defer stored_clear()
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
@(require_results)
get_string :: proc(format: string, args: ..any) -> string {
    fmt.printf(format, ..args)
    input := readline(&stdin.reader)
    return stored_push(input)
}

/* 
#### Brief:
Wrapper around `cs50.get_string` which reinterprets the input as an `int`.

#### Returns:
The result of `strconv.parse_int`. Will loop until it receives a valid `int`.

#### Notes:
See `cs50.get_string` for more detailed information.

Invalid strings have their memory freed during the loop, to reduce memory wastage.
 */
 @(require_results)
get_int :: proc(format: string, args: ..any) -> int {
    for {
        value, ok := strconv.parse_int(get_string(format, ..args))
        if ok do return value
        stored_pop_io_string() // User won't be able to use this line of text anyway
    }
}

/* 
#### Brief:
Wrapper around `cs50.get_string` which reinterprets the input as a `f64`.

#### Returns:
The result of `strconv.parse_f64`. We'll loop until a valid `f64` is received.

#### Notes:
See `cs50.get_string` for more detailed information.

Invalid strings have their memory freed during the loop, to reduce memory wastage.
 */
 @(require_results)
get_float :: proc(format: string, args: ..any) -> f64 {
    for {
        value, ok := strconv.parse_f64(get_string(format, ..args))
        if ok do return value
        stored_pop_io_string()
    }
}

/* 
#### Brief:
Wrapper around `readfile`. By itself, `readfile` doesn't push to internal storage. 

So this function takes care of that for you.

#### Note:
If you want to manage the memory of the string array, just call `readfile` as is.

You'll have to free all elements of the array along with the array itself.
 */
@(require_results)
get_file_view :: proc(file_path: string) -> File_View {
    file_view, _ := readfile(file_path)
    return stored_push(file_view)
}

@(require_results)
readline :: proc(reader: ^bufio.Reader) -> string {
    output: string
    for {
        line, err := bufio.reader_read_string(reader, '\n'); 
        if err != nil && err != io.Error.None {
            break
        }
        defer delete(line)
        output = strings.concatenate({output, line})
        if strings.contains_any(output, "\r\n") {
            output = strings.trim_right(output, "\r\n")
            break
        }
    }
    return output
}

/* 
#### Brief:
This is the iterator approach.

#### Links:
<https://odin-lang.org/news/read-a-file-line-by-line/>
 */
readline_from_handle :: proc(handle: os.Handle) -> string {
    data, ok := os.read_entire_file(handle)
    if !ok {
        return ""
    }
    defer delete(data)
    contents := string(data)
    return strings.clone(contents) // `contents` itself will go out of scope!
}

/* 
#### Brief:
This is the buffered IO approach.
 */
readline_from_reader :: proc(reader: ^bufio.Reader) -> string {
    final: string
    for {
        line, err := bufio.reader_read_string(reader, '\n')
        if err != nil && err != io.Error.None {
            break
        }
        defer delete(line)
        final = strings.concatenate({final, line})
        if strings.contains_any(final, "\r\n") {
            final = strings.trim_right(final, "\r\n")
            break
        }
    }
    return final
}

/* 
#### Brief:
Using `file_path`, open the file of the same name/path and store its contents
into a dynamic string array. This uses the iterative approach.

#### Return:
A slice, a.k.a. a view into the dynamic array. You have to free it yourself.

#### Links:
1. <https://odin-lang.org/news/read-a-file-line-by-line/>
 */
@(require_results)
readfile :: proc(file_path: string) -> (fv: File_View, err: mem.Allocator_Error) {
    file_data, success := os.read_entire_file(file_path)
    if !success {
        fmt.eprintf("Could not open file '%s'.\n", file_path) 
        return
    }
    defer delete(file_data)
    // See: https://odin-lang.org/docs/overview/#or_return-operator
    fv.buffer, err = strings.clone(string(file_data))
    if err != nil {
        fmt.eprintf("Could not clone data for '%s'.\n", file_path)
        return
    }
    fv.lines, err = strings.split_lines(fv.buffer)
    if err != nil {
        defer delete(fv.buffer)
        fmt.eprintf("Could not split lines to array for '%s'.\n", file_path)
        return
    }
    return
}
