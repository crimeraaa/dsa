package toarray

import "core:bufio"
import "core:fmt"
import "core:os"
import "core:strings"

main :: proc() {
    fs_test("I am the walrus")
    fs_test("The quick brown fox jumps over the lazy dog.")
    fs_test("She sells seashells by the seashore.")
    // ff_test("./sample.txt")
}

collection_deinit :: proc(collection: []string) {
    defer delete(collection)
    for item in collection do delete(item)
}

fs_test :: proc(src: string) {
    collection := from_string_iterated(src)
    defer if collection != nil do collection_deinit(collection)
    fmt.println(len(collection), collection[:])
}

ff_test :: proc(fpath: string) {
    collection := from_file_iterated(fpath)
    defer if collection != nil do collection_deinit(collection)
    fmt.println(len(collection), collection[:])
}

get_filename :: proc(filepath: string) -> string {
    start := strings.last_index_any(filepath, "/\\")
    stop  := len(filepath)
    start = (start + 1) if (start != -1 && start < stop) else 0
    return filepath[start:stop]
}

logprintln :: proc(caller := #caller_location) {
    // See: https://pkg.odin-lang.org/base/runtime/#Source_Code_Location
    fmt.printf("%s(%i,%i): %s\n", 
                get_filename(caller.file_path), 
                caller.line,
                caller.column,
                caller.procedure)
}

from_string_iterated :: proc(src: string, sep: string = " ") -> []string {
    logprintln()
    src := src
    buffer: [dynamic]string
    for str in strings.split_iterator(&src, sep) {
        // Since substrings are just views into `src`, we explicitly clone them.
        // This extends lifetime of each array item after `src` is long gone.
        append(&buffer, strings.clone(str))
    }
    return buffer[:]
}

from_file_iterated :: proc(path: string) -> []string {
    logprintln()
    fdata, success := os.read_entire_file(path)
    if !success {
        return nil
    }
    defer delete(fdata)
    return from_string_iterated(string(fdata), "\n") // string(fdata) goes out of scope!
}

from_file_buffered :: proc(fpath: string) -> []string {
    logprintln()
    fhandle, ferr := os.open(fpath)
    if ferr != os.ERROR_NONE {
        fmt.eprintf("ERROR: Failed to open file '%s'!\n", fpath)
        return nil
    }
    defer os.close(fhandle)
    
    freader: bufio.Reader
    fbuffer: [256]u8
    bufio.reader_init_with_buf(&freader, os.stream_from_handle(fhandle), fbuffer[:])
    defer bufio.reader_destroy(&freader)
    output: [dynamic]string
    for {
        line, lerr := bufio.reader_read_string(&freader, '\n')
        if lerr != nil {
            break // Useful when EOF reached
        }
        // NOTE: Don't `delete(line)` as we'll append them to `output`!
        append(&output, strings.trim_right(line, "\r\n"))
    }
    return output[:] // To return dyarrays, you gotta return slices.
}
