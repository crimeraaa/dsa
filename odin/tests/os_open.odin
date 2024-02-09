package tests

import "core:fmt"
import "core:os"
import "core:math/rand"

FILE_NAMES: []string = {"./sample.txt", "./invalid.txt"}

/* view memory usage with `valgrind` */
main :: proc() {
    defer delete(os.args)
    if len(os.args) != 2 && len(os.args) != 3 {
        print_usage()
        return
    }
    // thank heavens we can do this instead of a stupid if-else ladder
    switch os.args[1] {
    case "open": // lol
        file_path := rand.choice(FILE_NAMES[:]) if len(os.args) != 3 else os.args[2]
        try_os_open(file_path)
    case "read": 
        try_os_read()
    case:  
        fmt.eprintf("Invalid option '%s'.\n", os.args[1])
        fallthrough
    case "help": 
        print_usage()
    }
}

print_usage :: proc() {
    fmt.eprintf("Usage: %s <open [file]|read>\n", os.args[0])
    fmt.eprintln("<open> will default flipping a coin to open either sample.txt or invalid.txt.")
}

try_os_open :: proc(file_path: string) {
    fhandle, ferr := os.open(file_path)
    if ferr != os.ERROR_NONE {
        fmt.eprintf("Failed to open file '%s'.\n", file_path)
        return
    }
    defer os.close(fhandle)
    fmt.eprintf("Successfully opened file '%s'!\n", file_path)
}

/* https://discord.com/channels/568138951836172421/568871298428698645/1205331189771145248 */
try_os_read :: proc() {
    fmt.printf("Press (Enter) to exit > ")
    buf: [1]byte
    bytes_read, err := os.read(os.stdin, buf[:])
    fmt.println(bytes_read, err)
}
