#include "simple.h"

char *byte_to_bits(ga_byte byte, char *buffer)
{
    for (size_t i = 0; i < CHAR_BIT; i++) {
        size_t index = (CHAR_BIT - 1) - i; // Hack to write buffer correctly.
        buffer[index] = (byte & 1) ? '1' : '0'; // 1101 & 0001 = 0001
        byte >>= 1; // 1101 & 1 = 0110, throws away rightmost bit.
    }
    return buffer;
}

void dump_bytes(const void *p_memory, size_t n_sizeof)
{
    const ga_byte *p_bytes = p_memory; // Use type punning to interpret bytes.
    for (size_t i = 0; i < n_sizeof; i++) {
        ga_byte byte = p_bytes[i]; // Copy by value as need to mutate this.
        char buffer[CHAR_BIT + 1] = {0}; // Add 1 for nul
        printf("0x%02X = %s\n", byte, byte_to_bits(byte, buffer));
    }
    putchar('\n');
}

