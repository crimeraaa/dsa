package cs50

import "core:bufio"
import "core:log"
import "core:io"
import "core:os"
import "core:strings"

@(private)
Stored :: struct {
    user_iostrings: [dynamic]string,
    user_filelines: [dynamic][]string,
}

@(private)
stored: Stored

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
    defer bufio.reader_destroy(&stdin.reader)
    defer delete(stored.user_filelines)

    clear_array(stored.user_iostrings[:])
    for array in stored.user_filelines {
        clear_array(array[:])
    }
}

@(private)
clear_array :: proc(collection: []string) {
    defer delete(collection)
    for item in collection do delete(item)
}

@(require_results)
readline :: proc(reader: ^bufio.Reader) -> string {
    output: string
    for {
        line, lerr := bufio.reader_read_string(reader, '\n'); 
        if lerr != io.Error.None {
            break
        }
        defer delete(line)
        output = strings.concatenate({output, line})
        if strings.contains_any(output, "\r\n") {
            output = strings.trim_right(output, "\r\n")
            append(&stored.user_iostrings, output)
            break // Don't append if lerr is not nil
        }
    }
    return output
}

@(require_results)
readfile :: proc(filepath: string) -> []string {
    filedata, success := os.read_entire_file(filepath)
    if !success {
        log.errorf("Could not open file '%s'.\n", filepath) 
        return nil
    }
    defer delete(filedata)

    iterator := string(filedata)
    filelines: [dynamic]string
    /* 
    RATIONALE:
    `line` is just a view into `iterator`. When `line` goes out of
    scope (e.g: it's a string literal), we'll have views into invalid memory!
     */
    for line in strings.split_lines_iterator(&iterator) {
        copy, err := strings.clone(line)
        if err != nil {
            log.errorf("Failed to clone string \"%s\".\n", line)
            clear_array(filelines[:])
            return nil
        }
        append(&filelines, copy)
    }
    fileview := filelines[:]
    append(&stored.user_filelines, fileview)
    return fileview
}
