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
#### Brief:
Similar to the C standard library's `fgets`. No allocations are made within here.

#### Note:
By passing in a `slice`, we should (in theory) avoid heap allocations from within 
this function. This is because `line` is just a view into `buffer`.
*/
fixed_get_string :: proc(buffer: []byte, format: string, args: ..any) -> (line: string) {
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

/* 
#### Brief:
CS50-style `get_string`. We read a line of text from the standard input stream.

We consider a "line" to be any sequence of UTF-8 characters followed by a newline
character, such as CR, LF, or CRLF.

#### Notes:
The resulting string is a slice/a view into heap-allocated memory.
You are responsible for freeing it.
 */
get_string :: proc(format: string, args: ..any) -> (line: string) {
    fmt.printf(format, ..args)
    return readline(os.stdin)
}

/* 
#### Brief:
CS50-style `get_char`, but with no heap-allocations, and works with UTF-8. 

Reprompts if the given input was not a single Odin `rune` (a UTF-8 codepoint).
*/
get_rune :: proc(format: string, args: ..any) -> (letter: rune) {
    // Exactly enough memory for any single UTF8 codepoint with Windows CRLF.
    buffer: [utf8.UTF_MAX + 2]byte 
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
        // len(dummy) is in bytes, so it can't tell if we have multibyte chars.
        if strings.rune_count(dummy) == 1 {
            letter = utf8.rune_at_pos(dummy, 0)
            return
        }
    }
}

/* 
#### Brief:
Reads a line of text from the given handle or stream/reader.
We stop reading once a newline of either CR, LF, or CRLF is found.

#### Notes:
The returned string is just a view into an underlying heap-allocated buffer.
You are responsible for `delete`'ing it when done, as that frees the buffer.

`io.Reader` is a non-distinct alias for `io.Stream`, but I recommend that you
use `io.to_reader(stream)` before calling `readline_from_reader`.

*/
readline :: proc {
    readline_from_handle,
    readline_from_reader,
}

@(require_results)
readline_from_handle :: proc(handle: os.Handle) -> (line: string) {
    // return readline_from_stream(os.stream_from_handle(handle))
    stream := os.stream_from_handle(handle)
    reader, _ := io.to_reader(stream) // Need separate statement due to returned bool
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
        // No `contains_byte` proc so this will have to do.
        if strings.contains_rune("\r\n", rune(ch)) {
            break
        }
        append(&buffer, ch)
    }
    line = string(buffer[:])
    return
}

/* 
#### Brief:
Store the byte contents of `file_name` into 1 giant heap-allocated string.

#### Notes:
Actually, `contents` is more like a string view into a dynamic `byte` buffer.
You are responsible for `delete`'ing it when done, as that also frees the buffer.

On error, `contents` will retain its default value which is an empty string.
 */
@(require_results)
readfile :: proc(file_name: string) -> (contents: string) {
    // `os.open` will call runtime temp allocator which allocates ~4MB.
    handle, fopen_err := os.open(file_name, os.O_RDONLY)
    if fopen_err != os.ERROR_NONE {
        fmt.eprintf("Failed to open '%s'.\n", file_name)
        return
    }
    defer os.close(handle)
    
    // `os.file_size_from_path` opens a new handle == more memory used!
    file_size, fsize_err := os.file_size(handle)
    if fsize_err != os.ERROR_NONE {
        fmt.eprintf("Failed to get file size of '%s'.\n", file_name)
        return
    }

    // We already know the exact amount needed for our buffer, in bytes.
    // Verify with: `du --bytes -- <file>`
    buffer := make([dynamic]byte, file_size) 
    bytes_read, fread_err := os.read(handle, buffer[:])

    // Need to "cast" `bytes_read` as Odin doesn't do any implicit conversions.
    if (fread_err != os.ERROR_NONE) || (i64(bytes_read) != file_size) {
        defer delete(buffer)
        fmt.eprintf("Failed to read '%s'.\n", file_name)
        fmt.eprintf("Errno %i: read %i bytes out of %s's %i bytes.\n", 
                     fread_err, bytes_read, file_name, file_size)
        return
    }
    return string(buffer[:bytes_read])
}
