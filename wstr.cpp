#include "common.h"
#include "wstr.h"

// don't forget to call: setlocale(LC_ALL, "zh_CN.UTF-8");

std::string ws2s(const std::wstring &wstr) // convert wstring to string
{
    const wchar_t *src = wstr.c_str();
    char *dest = wcs2cs(src);
    std::string ret(dest);
    delete[] dest;
    return ret;
}

std::wstring s2ws(const std::string &str) // convert string to wstring
{
    const char *src = str.c_str();
    wchar_t *dest = cs2wcs(src);
    std::wstring ret(dest);
    delete[] dest;
    return ret;
}

char *wcs2cs(const wchar_t *wstr) // wide-cstring to cstring, alloc memory
{
    size_t len = wcstombs(NULL, wstr, 0);
    if (len == (size_t) -1) fail("wcstombs failed");
    char *ret = new char[len + 1];
    wcstombs(ret, wstr, len + 1);
    return ret;
}

wchar_t *cs2wcs(const char *str) // cstring to wide-cstring, alloc memory
{
    size_t len = mbstowcs(NULL, str, 0);
    if (len == (size_t) -1) fail("mbstowcs failed");
    wchar_t *ret = new wchar_t[len + 1];
    mbstowcs(ret, str, len + 1);
    return ret;
}
