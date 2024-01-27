#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Far as I know, even 128-bit unsigneds don't have this many digits.
 * We add 2 because: 1 slot for newline, another slot for nul char.
 */
#define STUPID_BUFFER_LENGTH ((sizeof(CHAR_BIT) * sizeof(size_t)) + 2)

int get_length(int arg, int base)
{
    int count = 0;
    if (arg < 0) {
        arg = abs(arg);
    }
    while (arg > 0) {
        arg /= base;
        count++;
    }
    return count;
}

/**
 * @brief   Reverse a C-string in-place.
 */
char *reverse_string(char *buffer, int length)
{
    for (int x = 0, y = length - 1; x < y; x++, y--) {
        char ch = buffer[x];
        buffer[x] = buffer[y];
        buffer[y] = ch;
    }
    return buffer;
}

void print_decimal(int arg)
{
    static const int base = 10;
    char buffer[STUPID_BUFFER_LENGTH] = {0}; // Init w/ 0's to nul terminate
    int length = 0; // number of chars in `buffer`
    bool negative = false;

    if (arg == 0) {
        fputc('0', stdout);
        return;
    }    

    if (arg < 0) {
        arg = abs(arg); // Lets us compare > 0
        negative = true;
    }

    // PROBLEM: Prints right to left!!!
    while (arg > 0) {
        int digit = arg % base; // Modulo gives us the rightmost digit
        char ch = digit + '0'; // Probably not portable, but works for now
        buffer[length++] = ch;
        arg /= base; // Throw away rightmost digit
    }
    if (negative) {
        buffer[length++] = '-';
    }
    buffer[length] = '\n'; // Don't increment so length - 1 is last digit
    fputs(reverse_string(buffer, length), stdout);
}

int main(void)
{
    print_decimal(123);
    print_decimal(123456789);
    print_decimal(0xFFFF);
    return 0;
}
