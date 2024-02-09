package custom_io

count_digits :: proc(value: int, base: int = 10) -> int {
    value := value // TODO: Handle overflow, e.g in C: abs(INT_MIN)
    count := 0
    for value > 0 {
        // rdigit := value % base // Modulo by base always gets rightmost digit
        count += 1
        value /= base // Division by base removes rightmost digit
    }
    return count
}

count_bin_digits :: proc(value: int) -> int {
    return count_digits(value, 2)
}

count_oct_digits :: proc(value: int) -> int {
    return count_digits(value, 8)
}

count_dec_digits :: proc(value: int) -> int {
    return count_digits(value, 10)
}

count_hex_digits :: proc(value: int) -> int {
    return count_digits(value, 16)
}
