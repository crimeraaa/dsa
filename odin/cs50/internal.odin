// See: https://odin-lang.org/docs/overview/#private
// This marks ALL types and procedures in the package as `@(private="package")`.
//+private
package cs50 

import "core:bufio"
import "core:fmt"
import "core:io"
import "core:os"

/* 
### Internal Implementation
This is meant to make reading from and writing to streams more unified.

e.g. we no longer have to constantly type this:
```odin
readline :: proc() {
    handle: os.stdin
    stream: os.stream_from_handle(os.stdin)
    buffer: [256]byte
    reader: bufio.Reader
    bufio.init_reader_with_buf(&reader, stream, buffer[:])
    defer bufio.reader_destroy(&reader)
    ...
}
```
 */
Wrap_IO :: struct {
    handle: os.Handle,
    stream: io.Stream,
    flags: int,
    buffer: [256]byte,
    reader: bufio.Reader,
    writer: bufio.Writer,
}

wrap_io_init :: proc {
    wrap_io_init_from_handle,
    wrap_io_init_from_filename,
}

wrap_io_init_from_handle :: proc(inst: ^Wrap_IO, handle: os.Handle, flags: int) {
    inst.handle = handle
    inst.stream = os.stream_from_handle(inst.handle)
    inst.flags = flags
    wrap_io_set_flags(inst)
}

wrap_io_init_from_filename :: proc(inst: ^Wrap_IO, filename: string, flags: int) {
    handle, errno := os.open(filename)
    if errno != os.ERROR_NONE {
        fmt.eprintf("Failed to open file '%s'.\n", filename)
        return
    }
    defer os.close(handle)
    wrap_io_set_flags(inst)
}

wrap_io_set_flags :: proc(inst: ^Wrap_IO) {
    wrap_io_use_flags(inst, wrap_io_callback_set_flags)
}

wrap_io_deinit :: proc(inst: ^Wrap_IO) {
    wrap_io_use_flags(inst, wrap_io_callback_deinit)
} 

/* 
Use bitmasks to allow user to also specify `os.O_RDWR`.
 */
wrap_io_use_flags :: proc(inst: ^Wrap_IO, callback: proc(inst: ^Wrap_IO, flag: int)) {
    callback(inst, inst.flags & os.O_RDONLY)
    callback(inst, inst.flags & os.O_WRONLY)
}
    
wrap_io_callback_set_flags :: proc(inst: ^Wrap_IO, flag: int) {
    switch flag {
    case os.O_RDONLY:
        bufio.reader_init_with_buf(&inst.reader, inst.stream, inst.buffer[:])
    case os.O_WRONLY:
        bufio.writer_init_with_buf(&inst.writer, inst.stream, inst.buffer[:])
    case:
        fmt.eprintf("Invalid flag %i, must be: O_RDONLY/O_WRONLY/O_RDWR.\n", flag)
    }
}

wrap_io_callback_deinit :: proc(inst: ^Wrap_IO, flag: int) {
    switch flag {
    case os.O_RDONLY:
        defer bufio.reader_destroy(&inst.reader)
    case os.O_WRONLY:
        defer bufio.writer_destroy(&inst.writer)
    case:
        fmt.eprintf("Invalid flag %i, must be: O_RDONLY/O_WRONLY/O_RDWR.\n", flag)
    }
}

/* 
### Internal Implementation
This is meant to make the freeing of memory allocated by this package unified.

This affects strings/arrays allocated by the `cs50.get_*` family of functions.

#### Note:
If you use `cs50.readline` or `cs50.readfile` as is, memory allocated by these
is your responsibility!
 */
Stored :: struct {
    io_strings: [dynamic]string,
    file_views: [dynamic]File_View,
}

allocated: Stored

stored_push :: proc {
    stored_push_io_string,
    stored_push_file_view,
}

stored_push_io_string :: proc(element: string) -> string {
    append(&allocated.io_strings, element)
    return element
}

stored_push_file_view :: proc(element: File_View) -> File_View {
    append(&allocated.file_views, element)
    return element
}

stored_pop_io_string :: proc() {
    item, ok := pop_safe(&allocated.io_strings) // See base:builtin
    defer if ok do delete(item)
}

stored_clear :: proc() {
    defer stored_clear_array(allocated.io_strings[:])
    defer delete(allocated.file_views)
    for fv in allocated.file_views {
        defer delete(fv.buffer)
        defer delete(fv.lines)
    }
}

stored_clear_array :: proc(collection: []string) {
    defer delete(collection)
    for item in collection do delete(item)
}
