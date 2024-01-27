#include <cstdarg>
#include <cstdio>
#include <string>

template<typename CharT>
using FmtFn = int(*)(CharT *, size_t, const CharT *, std::va_list);

/**
 * @brief   Works only for types `char` and `wchar_t`. 
 *          For Linux, `wchar_t` won't work. 
 *          See the specialization for `wchar_t` on Linux.
 * @tparam  CharT   One of `char` or `wchar_t`.
 * @tparam  FmtFn   One of `vsnprintf` or `vswprintf`, depending on `CharT`.
 *
 * @param   fmts    C-style format string with appropriate specifiers, if any.
 * @param   ...     Arguments to the format stirng.
 *
 * @return  `std::basic_string<CharT>` of your formatted output.
 */
template<typename CharT, FmtFn<CharT> fn>
std::basic_string<CharT> basic_format(const CharT *fmts, ...)
{
    std::basic_string<CharT> writer;
    std::va_list args, copy;
    int n_length;

    va_start(args, fmts);
    va_copy(copy, args);

    n_length = fn(nullptr, 0, fmts, args);
    if (n_length < 0) {
        goto cleanup;
    }
    writer.reserve(n_length + 1);
    writer.resize(n_length);
    n_length = fn(writer.data(), writer.capacity(), fmts, copy);
    if (n_length < 0) {
        std::basic_string<CharT> none;
        writer.swap(none);
        goto cleanup;
    }
cleanup:
    va_end(args);
    va_end(copy);
    return writer;
}

#ifndef _MSC_VER
#include "filehandle.hpp"
static FileHandle nulldevice("/dev/null");

/**
 * Specialization for Linux since `vswprintf(nullptr, 0, ...)` is invalid.
 * On Windows, though, that call is still valid.
 */
template<> 
std::wstring basic_format<wchar_t, vswprintf>(const wchar_t *fmts, ...)
{
    std::wstring ws;
    std::va_list args, copy;
    int n;
    va_start(args, fmts);
    va_copy(copy, args);
    n = vfwprintf(nulldevice.get_fileptr(), fmts, args);
    if (n < 0) {
        goto cleanup;
    }
    ws.reserve(n + 1);
    ws.resize(n);
    n = vswprintf(ws.data(), ws.capacity(), fmts, copy);
    if (n < 0) {
        std::wstring none;
        ws.swap(none);
        goto cleanup;
    }
cleanup:
    va_end(args);
    va_end(copy);
    return ws;
}
#endif

constexpr const auto csformat = basic_format<char, vsnprintf>;
constexpr const auto wcsformat = basic_format<wchar_t, vswprintf>;

void print_csformat(const std::string &s)
{
    std::printf("s=\"%s\",length=%zu,capacity=%zu\n",s.c_str(),s.length(),s.capacity());
}

void print_wcsformat(const std::wstring &ws)
{
    std::printf("ws=\"%ls\",length=%zu,capacity=%zu\n",ws.c_str(),ws.length(),ws.capacity());
}

int main() 
{
    auto s = csformat("Hi %s! Today is %li/%i/%hi.", "mom", 2024L, 1, short{25});
    auto ws = wcsformat(L"Hi %s! It's %.2f degrees out.", "mom", 27.0f);
    print_csformat(s);
    print_wcsformat(ws);
    return 0;
}
