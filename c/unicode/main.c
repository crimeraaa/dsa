#include <locale.h> /* setlocale */
#include <limits.h> /* MB_LEN_MAX */
#include <stdio.h> /* fput*, fget*, put* families */
#include <stdlib.h> /* wctomb, wcstombs, */
#include <string.h> /* str* family */
#include <wchar.h> /* wchar_t, fputw*, fgetw*, putw*, wcs* families*/

/**
 * @brief   When locale is `"en_US.UTF-8"`, we can print `wchar_t` as stored
 *          inside of `char` arrays thanks to `wctomb`!
 */
void print_wctomb(wchar_t lc)
{
    char ch[MB_LEN_MAX] = {0}; // valid for any of this system's encodings
    ch[wctomb(ch, lc)] = '\0'; // multibyte unicode string even if only 1 wchar_t
    printf("wchar_t lc = L'%lc' (len = 1)\n", lc);
    printf("char ch[%i] = \"%s\" (len = %zu)\n\n", MB_LEN_MAX, ch, strlen(ch));
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
    size_t n_length = wcslen(ls) + 1; // Add 1 for nul char.
    size_t n_bytes = sizeof(*ls) * n_length; // Maybe overkill but I'm paranoid
    char *s = malloc(n_bytes);
    if (!s) {
        perror("failed to allocate memory!");
        goto deinit;
    }
    printf("const wchar_t *ls = L\"%ls\" (len = %zu)\n", ls, n_length);
    n_length = wcstombs(s, ls, n_bytes); // Also copies over nul char :)
    if (n_length == (size_t)-1) {
        perror("failed to convert some wide character!");
        goto deinit;    
    }
    printf("char *s = \"%s\" (len = %zu)\n", s, n_length);
    printf("is correct strlen? %s\n\n", (strlen(s) == n_length) ? "yes" : "no");
deinit:
    free(s);
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
