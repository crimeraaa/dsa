#include "simple.h"

void totally_safe(double f)
{
    long li = *(long*)&f; // Type punning is my fav :)
    printf("f = %f, li = %li, seems legit\n", f, li);
}

int main(void)
{
    short hi = 16789;
    int i = 13;
    char c = 'A';
    wchar_t wc = L'B';
    size_t size = 0xFEEDBEEF;
    const char *s = "Hi mom!";
    dump_bytes(&hi, sizeof(hi));
    dump_bytes(&i, sizeof(i));
    dump_bytes(&c, sizeof(c));
    dump_bytes(&wc, sizeof(wc));
    dump_bytes(&size, sizeof(size));
    dump_bytes(s, strlen(s) + 1);
    totally_safe(1.3);
    totally_safe(0.0);
    return 0;
}
