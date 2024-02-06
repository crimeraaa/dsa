package test

import "core:bufio"
import "core:fmt"   // Odin already looks in its build directory
import "core:io"
import "core:os"
import "core:strings"
// import "../cs50"    // Relative path seems okay

// Use so we don't have to constantly rebuild readers for stdin and firends.
StreamReader :: struct {
    handle: os.Handle,
    stream: io.Stream,
    buffer: [16]byte,
    reader: bufio.Reader,
}

main :: proc() {
    sample_readfile("./sample.txt")
    name := sample_get_string("Enter your name: ")
    pets := sample_get_string("Hi %s! What's your favorite pets? ", name)
    defer delete(name)
    defer delete(pets)
    fmt.printf("name = \"%s\", pets = \"%s\"\n", name, pets)
}

get_streamreader :: proc(handle: os.Handle) -> StreamReader  {
    inst: StreamReader
    inst.handle = handle
    inst.stream = os.stream_from_handle(handle)
    bufio.reader_init_with_buf(&inst.reader, inst.stream, inst.buffer[:])
    return inst
}

// Uses a TON of memory! Also seems that returning dynamic arrays is hard.
sample_readfile :: proc(filepath: string) {
    final := make([dynamic]string, 0, 16) // https://odin-lang.org/docs/overview/#dynamic-arrays
    defer delete(final)
    data, success := os.read_entire_file(filepath)
    if !success {
        return
    }
    defer delete(data)
    it := string(data) // byte[] -> string
    // Iterate over every substring ending in a newline.
    for line in strings.split_lines_iterator(&it) {
        append(&final, line)
    }
    
    for line, index in final {
        fmt.printf("%i: %s\n", index, line)
    }
    return
}
