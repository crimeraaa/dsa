/**
 * NOTE: This is a horrible horrible idea! This is just for fun!
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char byte;

typedef struct _horrible_array {
    byte *m_buffer;
    size_t m_count; // Number of elements currently written to the buffer.
    size_t m_capacity; // Number of elements buffer was allocated for.
    size_t m_sizeof; // Result of `sizeof` for the array's intended type.
} horrible_array;

typedef struct _args_ha_init {
    size_t size;
    size_t count;    
} args_ha_init;

horrible_array ha_init(args_ha_init args)
{
    horrible_array inst = {
        .m_buffer   = NULL,
        .m_count    = args.count,
        .m_capacity = 0,
        .m_sizeof   = args.size,
    };
    return inst;
}

void ha_print_info(const horrible_array *ha)
{
    printf(
        "ha->m_buffer = %p\n"
        "ha->m_count = %zu\n"
        "ha->m_capacity = %zu\n"
        "ha->m_sizeof = %zu\n" ,
        (void*)ha->m_buffer,
        ha->m_count,
        ha->m_capacity,
        ha->m_sizeof
    );
}

/**
 * Hacky macro that allows for "named" arguments of sorts.
 * By using a struct to "pack" out arguments, we can also make default values.
 * 
 * Of course whether or not this is worth the trouble is up to you.
 * 
 * @param   .size   `sizeof` for your desired element.
 * @param   .count  How many elements we should allocate for at first. Default: 0
 * 
 * @note    A: Reassigning the value of a struct member, even after it's
 *          already been assigned previously, is allowed by the standard.
 * 
 * @note    B: See the following links:
 *          1. https://stackoverflow.com/a/5947763
 *          2. https://stackoverflow.com/a/48264760
 *          3. https://stackoverflow.com/a/2926165
 *          
 * @warning If you're absolutely certain about going down this path, you may
 *          want to pass `-Wno-override-init`.
  */
#define ha_init(...) ha_init((args_ha_init){.count = 0, __VA_ARGS__})

int main(void)
{
    horrible_array ha = ha_init(.size = sizeof(int), .count = 13);
    ha_print_info(&ha);
    return 0;
}
