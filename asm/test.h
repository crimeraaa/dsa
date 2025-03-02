#pragma once

#define STDIN  0
#define STDOUT 1

typedef long long           ssize_t;
typedef unsigned long long  size_t;

struct String {
    const char *data;
    size_t      len;
};

typedef struct String String;

// Defined in `cstring.asm`.
extern size_t
cstring_len(const char *cstring);

// Defined in `io.asm`.
extern ssize_t
read(int fd, void *buf, size_t count);

extern ssize_t
write(int fd, const void *buf, size_t count);

// Defined in `string.asm`.
extern _Bool
string_eq(String a, String b);

extern int
mem_compare(const void *buf1, const void *buf2, size_t n);
