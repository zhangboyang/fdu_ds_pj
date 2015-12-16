#ifndef ZBY_WSTR_H
#define ZBY_WSTR_H
#include "common.h"

#include <cstdlib>
#include <string>

std::string ws2s(const std::wstring &wstr);
std::wstring s2ws(const std::string &str);
char *wcs2cs(const wchar_t *wstr);
wchar_t *cs2wcs(const char *str);

#endif
