#ifndef ZBY_WSTR_H
#define ZBY_WSTR_H
#include "common.h"

#include <cstdlib>
#include <string>

std::string ws2s(const std::wstring &wstr);
std::wstring s2ws(const std::string &wstr);

#endif
