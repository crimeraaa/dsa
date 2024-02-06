package hello

import "core:fmt"
import "core:os"      // os.stdin, os.read
import "core:strings" // Different from the global `string` package!

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
    buffer: [256]byte // For now, this should be good enough.
    nbytes, nerrno := os.read(os.stdin, buffer[:]) 
    if nerrno < 0 {
        return "Error reading from stdin :("
    }
    writer := string(buffer[:nbytes - 1]) // Slice of buf[0..(nbytes-1)]
    return strings.clone(writer) // Allocate a copy since `writer` is stack-only
}
