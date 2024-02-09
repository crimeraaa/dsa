#include <stdio.h>

#define FILE_NAME "./sample.txt"

int main(void)
{
    FILE *fp = fopen(FILE_NAME, "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open '%s'.\n", FILE_NAME);
        return 1;
    }
    fprintf(stderr, "Successfully opened '%s'!\n", FILE_NAME);
    fclose(fp);
    return 0;
}
