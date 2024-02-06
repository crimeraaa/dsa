package test

import "core:bufio"
import "core:fmt"
import "core:io"
import "core:os"
import "core:strings"

stdin: StreamReader

@(require_results)
sample_get_string :: proc(format: string, args: ..any) -> string{
    fmt.printf(format, ..args)
    return helper_readline(&stdin.reader)
}

@(require_results)
helper_readline :: proc(reader: ^bufio.Reader) -> string {
    final: string
    for {
        line, err := bufio.reader_read_string(reader, '\n')
        if err != nil {
            fmt.eprintf("Failed to read from stream; err = %s\n", err)
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

// The `@(init)` attributes causes this function to be run before `main`.
@(init)
startup :: proc() {
    stdin.handle = os.stdin
    stdin.stream = os.stream_from_handle(stdin.handle)
    bufio.reader_init_with_buf(&stdin.reader, stdin.stream, stdin.buffer[:]) 
}

// This is allowed it seems but only when `main` is in the package scope.
@(deferred_none=main)
teardown :: proc() {
    bufio.reader_destroy(&stdin.reader)
}
