package hello

import "core:bufio"
import "core:fmt"
import "core:os"      // os.stdin, os.read
import "core:strings" // Different from the global `string` package!

stdin  := os.stream_from_handle(os.stdin)
stdout := os.stream_from_handle(os.stdout)
stderr := os.stream_from_handle(os.stderr)

/* 
Taken from:
```txt
1- https://odin-lang.org/news/read-a-file-line-by-line/
2- https://stackoverflow.com/q/76748903
```

Reference:
```txt
/home/crimeraaaa/source-code/Odin/core/strings/strings.odin
```

Do note that you need to delete the resulting string!
*/
get_string :: proc(prompt: string) -> string {
    fmt.print(prompt) // Don't print a newline.
    buffer: [16]byte // For now, this should be good enough.
    nbytes, nerrno := os.read(os.stdin, buffer[:]) 
    if nerrno < 0 {
        return "Error reading from stdin :("
    }
    writer := string(buffer[:nbytes - 1]) // Slice of buf[0..(nbytes-1)]
    return strings.clone(writer) // Allocate a copy since `writer` is stack-only
}

/* Adapted from: https://odin-lang.org/news/read-a-file-line-by-line/ */
super_get_string :: proc(prompt: string) -> string {
    reader: bufio.Reader
    buffer: [16]byte
    output: string
    defer delete(output) // strings.concatenate will allocate memory!!

    bufio.reader_init_with_buf(&reader, stdin, buffer[:])
    defer bufio.reader_destroy(&reader)
    
    fmt.print(prompt)
    for {
        line, err := bufio.reader_read_string(&reader, '\n'); 
        if err != nil {
            break
        }
        defer delete(line) // bufio.reader_read_string likely allocates memory.
        output = strings.concatenate({output, line})
        if strings.contains_any(output, "\r\n") {
            output = strings.trim_right(output, "\r\n")
            break
        }
    }

    return strings.clone(output)
}
