#include <locale.h> /* setlocale */
#include <limits.h> /* MB_LEN_MAX */
#include <stdio.h> /* fput*, fget*, put* families */
#include <stdlib.h> /* wctomb, wcstombs, */
#include <string.h> /* str* family */
#include <wchar.h> /* wchar_t, fputw*, fgetw*, putw*, wcs* families*/

/* Error return. See `man wctomb` or one of: `wcrtomb, wcstombs, wcsrtombs` */
#define WCTOMB_ERROR ((size_t)-1)

const wchar_t *g_strtests[] = {
    L"Hi mom!",
    L"\u00A1Hola Se\u00F1or y Se\u00F1orita!", // Spanish
    L"привет!", // Russian
    L"你好!", // Chinese (Simplified)
    L"สวัสดี!", // Thai
    L"नमस्ते!", // Hindi
    L"مرحبًا!", // Arabic
};

const size_t g_numtests = sizeof(g_strtests) / sizeof(*g_strtests);

/**
 * @brief   When locale is `"en_US.UTF-8"`, we can print `wchar_t` as stored
 *          inside of `char` arrays thanks to `wctomb`!
 *
 * @note    Heavily taken from glibc's implementation of vfprintf:
 *          https://github.com/lattera/glibc/blob/master/stdio-common/vfprintf.c
 */
void print_wctomb(wchar_t lc)
{
    char mbchar[MB_LEN_MAX] = {0}; // valid for any of this system's encodings
    mbstate_t mbstate = {0}; // shift state
    size_t len = wcrtomb(mbchar, lc, &mbstate); // Number of bytes written
    if (len == WCTOMB_ERROR) {
        perror("print_wctomb(): Failed to convert a wchar_t!");
        return;
    }
    printf("mbchar = %s (strlen = %zu)\n", mbchar, len);
}

/**
 * @brief   We can use `wcstombs` (wide character string to multi-byte string)
 *          to convert a `wchar_t*` to its individual raw bytes (in UTF-8) and
 *          store them in a heap-allocated `char` array.
 *
 * @note    Make sure `setlocale` is set to the languages you intend to support!
 *          e.g. `setlocale(LC_ALL, "")` allows you to use all possible encodings
 *          your system supports.
 */
void print_wcstombs(const wchar_t *ls)
{
    size_t len = wcslen(ls) + 1; // Add 1 for nul char.
    size_t bytes = sizeof(*ls) * len; // Maybe overkill but I'm paranoid
    mbstate_t mbstate = {0}; // Shift state
    char *mbstring = malloc(bytes);
    if (!mbstring) {
        perror("print_wcstombs(): failed to allocate memory!");
        goto deinit;
    }
    len = wcsrtombs(mbstring, &ls, bytes, &mbstate); // char-based strlen
    if (len == WCTOMB_ERROR) {
        perror("print_wcstombs(): failed to convert 1/more wchar_t's!");
        goto deinit;
    }
    printf("mbstring = \"%s\" (strlen = %zu)\n", mbstring, len);
deinit:
    free(mbstring);
}

typedef struct utf8_string {
    char *buffer;
    size_t length;
    size_t capacity;
} utf8_string;

utf8_string *u8s_init(utf8_string *self, size_t n_chars)
{
    if (self == NULL) {
        return NULL;
    }
    // More than enough memory for entire `wchar_t*` and its nul char
    size_t n_bytes = sizeof(wchar_t) * (n_chars + 1);
    self->buffer = malloc(n_bytes);
    self->length = n_chars; // Subject to change due to multibyte encoding
    self->capacity = n_bytes; // Should be consistent
    return (self->buffer) ? self : NULL;
}

void u8s_deinit(utf8_string *self)
{
    if (!self) {
        return;
    }
    free(self->buffer);
    self->buffer = NULL;
    self->length = 0;
    self->capacity = 0;
}

utf8_string u8s_from_wstring(const wchar_t *ls)
{
    mbstate_t mbstate = {0}; // Reset shift state
    utf8_string s; 
    if (u8s_init(&s, wcslen(ls)) == NULL) {
        perror("utf8string buffer could not be allocated");
        goto deinit;
    }
    // Would-be result of `strlen(s.buffer)` at this point
    s.length = wcsrtombs(s.buffer, &ls, s.capacity, &mbstate);
    if (s.length == WCTOMB_ERROR) {
        perror("failed to convert wchar_t* to char*");
        goto deinit;
    }
    return s;
deinit:
    u8s_deinit(&s);
    return s;
}

void u8s_print(const utf8_string *self)
{
    if (!self || !self->buffer) {
        return;
    }
    printf("\"%s\" (strlen = %zu)\n", self->buffer, self->length);
}

void test_chars(void)
{
    print_wctomb(L'A');
    print_wctomb(L'!');
    print_wctomb(0x2605); // U+2605: Black Star
    print_wctomb(0x00A1); // U+00A1: Inverted Exclamation Mark
    print_wctomb(0x00F1); // U+00F1: Latin Small Letter N with Tilde
}

void test_strings(void)
{
    for (size_t i = 0; i < g_numtests; i++) {
        print_wcstombs(g_strtests[i]);
    }
}

void test_utf8string(void)
{
    for (size_t i = 0; i < g_numtests; i++) {
        utf8_string s = u8s_from_wstring(g_strtests[i]);
        u8s_print(&s);
        u8s_deinit(&s);
    }
}

int main(void)
{
    // default is "C", this call *should* set locale to "en_US.UTF-8"
    if (setlocale(LC_CTYPE, "") == NULL) { 
        perror("setlocale failed, we need UTF-8 encoding");
        return 1;
    }
    // test_chars();
    // test_strings();
    test_utf8string();
    return 0;
}

