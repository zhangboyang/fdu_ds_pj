#ifdef ZBY_OS_WINDOWS
#include <windows.h>
#endif

#include "common.h"
#include "wstr.h"

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

#ifdef ZBY_OS_LINUX
// don't forget to call: setlocale(LC_ALL, "zh_CN.UTF-8");
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
#endif

#ifdef ZBY_OS_WINDOWS
char *wcs2cs(const wchar_t *wstr) // note: convert to wchar_t to UTF-8
{
    size_t len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    if (len == 0) fail("WideCharToMultiByte");
    char *ret = new char[len];
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, ret, len, NULL, NULL);
    return ret;
}

wchar_t *cs2wcs(const char *str) // cstring to wide-cstring, alloc memory
{
    size_t len = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    if (len == 0) fail("MultiByteToWideChar failed");
    wchar_t *ret = new wchar_t[len]; // dislike mbstowcs(), no +1 here
    MultiByteToWideChar(CP_UTF8, 0, str, -1, ret, len);
    return 0;
}
#endif
