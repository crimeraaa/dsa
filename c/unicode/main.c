#include <locale.h> /* setlocale */
#include <limits.h> /* MB_LEN_MAX */
#include <stdio.h> /* fput*, fget*, put* families */
#include <stdlib.h> /* wctomb, wcstombs, */
#include <string.h> /* str* family */
#include <wchar.h> /* wchar_t, fputw*, fgetw*, putw*, wcs* families*/

/* Error return. See `man wctomb` or one of: `wcrtomb, wcstombs, wcsrtombs` */
#define WCTOMB_ERROR ((size_t)-1)

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

int main(void)
{
    printf("old locale = %s\n", setlocale(LC_ALL, NULL)); // "C"
    printf("new locale = %s\n", setlocale(LC_CTYPE, "")); // "en_US.UTF-8"
    print_wctomb(L'A'); 
    print_wctomb(L'!');
    print_wctomb(0x2605); // U+2605: Black Star
    print_wctomb(0x00A1); // U+00A1: Inverted Exclamation Mark
    print_wctomb(0x00F1); // U+00F1: Latin Small Letter N with Tilde
    print_wcstombs(L"Hi mom!");
    print_wcstombs(L"\u00A1Hola Se\u00F1or y Se\u00F1orita!"); // Spanish
    print_wcstombs(L"привет!"); // Russian
    print_wcstombs(L"你好!"); // Chinese (Simplified)
    print_wcstombs(L"สวัสดี!"); // Thai
    print_wcstombs(L"नमस्ते!"); // Hindi
    print_wcstombs(L"مرحبًا!"); // Arabic (will not print right-to-left!)
    return 0;
}
