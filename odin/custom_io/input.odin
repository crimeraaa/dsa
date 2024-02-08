package custom_io

import "core:fmt"
import "core:io"
import "core:os"
import "core:strings"
import "core:unicode"
import "core:unicode/utf8"

/* 
Derived from:
0x0001_0fff => 0d69_631 => 0b0001_0000_1111_1111_1111 => 17 bits 

See: 
<https://stackoverflow.com/a/9533324>
*/
BITS_RUNE := count_bin_digits(int(unicode.MAX_RUNE))

/* 
By passing in a slice, we should (in theory) avoid heap allocations. 

This is because `line` is just a view into `buffer`.
*/
get_string_fixed :: proc(buffer: []byte, format: string, args: ..any) -> (line: string) {
    fmt.printf(format, ..args)
    bytes_read, errno := os.read(os.stdin, buffer[:]) 
    if errno != os.ERROR_NONE {
        return
    }
    // If we don't limit the slice, we could copy all 256 bytes of `buffer`!
    dummy := string(buffer[:bytes_read])
    endl  := strings.index_any(dummy, "\r\n")
    start := 0
    stop  := endl if endl > -1 else len(dummy)
    line = dummy[start:stop]
    return
}

get_string_dynamic :: proc(format: string, args: ..any) -> (line: string) {
    fmt.printf(format, ..args)
    return readline(os.stdin)
}

/* 
CS50-style `get_char`, but with no heap-allocations, and works with UTF-8. 

Reprompts if the given input was not a single Odin `rune` (UTF-8 `char`).
*/
get_rune :: proc(format: string, args: ..any) -> (letter: rune) {
    buffer: [utf8.UTF_MAX + 2]byte // More than enough memory for biggest UTF8 char w/ CRLF
    for {
        fmt.printf(format, ..args)
        bytes_read, errno := os.read(os.stdin, buffer[:])
        if errno != os.ERROR_NONE {
            return
        }
        dummy := string(buffer[:bytes_read])
        if strings.contains_any(dummy, "\r\n") {
            dummy = strings.trim_right(dummy, "\r\n")
        }
        // len(dummy) is in bytes, so it's hard to tell if we have multibytes.
        if strings.rune_count(dummy) == 1 {
            letter = utf8.rune_at_pos(dummy, 0)
            return
        }
    }
}

/* Note that `io.Reader` is just an alias for `io.Stream`. */
readline :: proc {
    readline_from_handle,
    readline_from_reader,
}

@(require_results)
readline_from_handle :: proc(handle: os.Handle) -> (line: string) {
    // return readline_from_stream(os.stream_from_handle(handle))
    stream := os.stream_from_handle(handle)
    reader, _ := io.to_reader(stream)
    return readline_from_reader(reader)
}

@(require_results)
readline_from_reader :: proc(reader: io.Reader) -> (line: string) {
    buffer: [dynamic]byte
    for {
        ch, err := io.read_byte(reader)
        if err != io.Error.None {
            break
        }
        if ch == '\r' || ch == '\n' {
            break
        }
        append(&buffer, ch)
    }
    line = string(buffer[:])
    return
}

readfile :: proc(file_name: string) -> (contents: string) {
    // Based on my Valgrind tests, `os.open` alone allocated 4.2mil bytes LOL
    handle, fopen_err := os.open(file_name, os.O_RDONLY)
    if fopen_err != os.ERROR_NONE {
        fmt.eprintf("Failed to open file '%s'.\n", file_name)
        return
    }
    defer os.close(handle)

    file_info, fstat_err := os.fstat(handle)
    if fstat_err != os.ERROR_NONE {
        fmt.eprintf("Failed to get file information for '%s'.\n", file_name)
        return
    }
    defer os.file_info_delete(file_info)

    // Init w/ len == file_info.size, useful when passing slices thereof
    // Verify with: `du --bytes -- <file>`
    buffer := make([dynamic]byte, file_info.size) 
    bytes_read, read_err := os.read(handle, buffer[:])
    if read_err != os.ERROR_NONE || i64(bytes_read) != file_info.size {
        defer delete(buffer)
        fmt.eprintf("Failed to read file '%s'.\n", file_name)
        return
    }
    return string(buffer[:bytes_read])
}
