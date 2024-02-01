#include "utf8.h"

/* Yes, we need 2 macros: https://stackoverflow.com/a/2670919 */
#define STRINGIFY_ALPHA(x) #x
#define STRINGIFY_BRAVO(x) STRINGIFY_ALPHA(x)
/* Create string literals prefixed with our file name and line number. */
#define TOLOCATION(msg) __FILE__ ":" STRINGIFY_BRAVO(__LINE__) ": " msg

/* See man pages for one of: `wctomb, wcstombs, wcrtomb, wcsrtombs` */
static constexpr size_t TO_MULTIBYTE_ERROR = static_cast<size_t>(-1);

/* See man pages for one of: `mbtowc, mbstowcs, mbrtowc, mbsrtowcs` */
static constexpr size_t TO_WIDECHAR_ERROR = static_cast<size_t>(-1);

int narrow_fputwc(int wc, FILE *stream)
{
    char mbchar[MB_LEN_MAX + 1] = {0};
    mbstate_t mbstate; // Shift state, need here to reset before call.    
    memset(&mbstate, 0, sizeof(mbstate)); // 0-init structs not allowed in C++
    size_t mblen = wcrtomb(mbchar, wc, &mbstate);
    if (mblen == TO_MULTIBYTE_ERROR) {
        perror(TOLOCATION("wcrtomb() failed"));
        return EOF;
    }
    return fputs(mbchar, stream);
}

int narrow_fputws(const wchar_t *ws, FILE *stream)
{
    int ret = EOF; // Initialize here due to use of `goto`.
    size_t mblen = wcslen(ws) + 1;
    size_t bytes = MB_CUR_MAX * mblen; // `MB_CUR_MAX` is a function call.
    mbstate_t mbstate; // Shift state, need here to reset before call.
    char *mbstring = static_cast<char*>(malloc(bytes));
    if (mbstring == nullptr) {
        perror(TOLOCATION("malloc() failed"));
        goto deinit;
    }
    memset(&mbstate, 0, sizeof(mbstate));
    // `ws` may be `0x0` after. Note how we pass `bytes`, not `mblen`.
    mblen = wcsrtombs(mbstring, &ws, bytes, &mbstate); 
    if (mblen == TO_MULTIBYTE_ERROR) {
        perror(TOLOCATION("wcsrtombs() failed"));
        goto deinit;
    }
    ret = fputs(mbstring, stream);
deinit:
    free(mbstring);
    return ret;
}

int wide_fputc(int c, FILE *stream)
{
    // Only 1 char but this function is around for naming consistency.
    // On Windows this is actually a demotion since sizeof(wchar_t) == 2.
    // Remember that the return value of `fputwc` is param `wc` itself.
    wint_t ret = fputwc(static_cast<wchar_t>(c), stream);
    return (ret != WEOF) ? static_cast<int>(ret) : EOF;
}

int wide_fputs(const char *s, FILE *stream)
{
    int ret = -1; // Inconsistent with `fputwc`, see `man fputws`.
    size_t mblen = strlen(s) + 1; // Even if `s` is multibyte, this is OK.
    size_t bytes = sizeof(wchar_t) * mblen;
    mbstate_t mbstate;
    wchar_t *mbwstring = static_cast<wchar_t*>(malloc(bytes));
    if (mbwstring == nullptr) {
        perror(TOLOCATION("malloc() failed"));
        goto deinit;
    }
    memset(&mbstate, 0, sizeof(mbstate));
    // Depends on the LC_CTYPE category of the current locale.
    mblen = mbsrtowcs(mbwstring, &s, bytes, &mbstate);
    if (mblen == TO_WIDECHAR_ERROR) {
        perror(TOLOCATION("mbsrtowcs() failed"));
        goto deinit;
    }
    ret = fputws(mbwstring, stream);
deinit:
    free(mbwstring);
    return ret;
}
