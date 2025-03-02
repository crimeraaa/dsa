#include "test.h"

#define print(literal)      write(STDOUT, literal, sizeof(literal) - 1)
#define println(literal)    print(literal "\n")

int
main(void)
{
    char buf1[64];
    char buf2[64];

    print(">>> ");
    ssize_t buf1_len = read(STDIN, buf1, sizeof buf1);
    buf1[buf1_len]   = '\0';

    print(">>> ");
    ssize_t buf2_len = read(STDIN, buf2, sizeof buf2);
    buf2[buf2_len]   = '\0';

    String a = {buf1, cstring_len(buf1)};
    String b = {buf2, cstring_len(buf2)};
    if (a.len == b.len && mem_compare(a.data, b.data, a.len) == 0)
        println("equal");
    else
        println("not equal");
    return 0;
}
