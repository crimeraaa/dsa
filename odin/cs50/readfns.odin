package cs50

import "core:bufio"   // .Reader, .reader_init_with_buf, .reader_read_string
import "core:io"      // .Stream
import "core:os"      // .stdin, .stream_from_handle,
import "core:strings" // .concatenate, .contains_any, .trim_right

// Helps reduce user-error somewhat? Don't expose this to users though.
@(private)
stored_strings: [dynamic]string

// A 1D array of string arrays, used by `readfile`.
@(private)
stored_arrays: [dynamic][dynamic]string

/* 
Must be called or deferred by users as we can't defer at global scope! 

e.g:
```odin
import "core:fmt"
import "cs50"

main :: proc() {
    defer cs50.clear_allocations()
    name = cs50.get_string("Enter your name: ")
    fmt.printf("Hi %s!\n", name) 
}
```
*/
clear_allocations :: proc() {
    for value in stored_strings {
        delete(value)
    }
    for array in stored_arrays {
        for value in array {
            delete(value)
        }
        delete(array)
    }
    delete(stored_strings) // Dynamic arrays themselves are heap-allocated!
    delete(stored_arrays)
}

// More on procedure attributes: <https://odin-lang.org/docs/overview/>
@(require_results)
readline :: proc(stream: io.Stream) -> string {
    reader: bufio.Reader // Helps us read data from `stream`.
    output: string // To be dynamically allocated by `.concatenate`.
    buffer: [256]u8 // Used when initializing `reader`. 256 seems reasonable.

    // .reader_init uses an internal buffer of 4096 which is way too much!
    bufio.reader_init_with_buf(&reader, stream, buffer[:])
    defer bufio.reader_destroy(&reader) // See `reader.odin`

    for line, err := bufio.reader_read_string(&reader, '\n'); err == nil; {
        defer delete(line) // .reader_read_string usually allocates memory.
        output = strings.concatenate({output, line})
        if strings.contains_any(output, "\r\n") {
            output = strings.trim_right(output, "\r\n")
            append(&stored_strings, output) // Don't append if err is not nil
            break
        }
    }
    return output
}

readfile :: proc(filepath: string) -> [dynamic]string {
    filedata, success := os.read_entire_file(filepath)
    if !success {
        return nil // Could not read file!
    }
    defer delete(filedata)

    iterator := string(filedata) // Convert raw bytes to one giant string
    filelines: [dynamic]string

    // Iterator over out giant string where we loop per newlines.
    for line in strings.split_lines_iterator(&iterator) {
        append(&filelines, line)
    }
    return filelines
}
