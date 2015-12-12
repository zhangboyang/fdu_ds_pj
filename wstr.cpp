#include "common.h"
#include "wstr.h"

// don't forget to call: setlocale(LC_ALL, "zh_CN.UTF-8");

std::string ws2s(const std::wstring &wstr) // convert wstring to string
{
    const wchar_t *src = wstr.c_str();
    size_t len = wcstombs(NULL, src, 0);
    if (len == (size_t) -1) fail("wcstombs failed");
    char *dest = new char[len + 1];
    wcstombs(dest, src, len + 1);
    std::string ret(dest);
    delete[] dest;
    return ret;
}

std::wstring s2ws(const std::string &wstr) // convert string to wstring
{
    const char *src = wstr.c_str();
    size_t len = mbstowcs(NULL, src, 0);
    if (len == (size_t) -1) fail("mbstowcs failed");
    wchar_t *dest = new wchar_t[len + 1];
    mbstowcs(dest, src, len + 1);
    std::wstring ret(dest);
    delete[] dest;
    return ret;
}

