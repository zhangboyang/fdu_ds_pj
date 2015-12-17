//#include <cstdio>
#include "common.h"
#include "MapSpecialTag.h"

MapSpecialTag::MapSpecialTag() { tid = 0; }
void MapSpecialTag::insert(const char *str)
{
    //printf("insert %s\n", str);
    const char *ptr;
    do {
        while (*str && isspace(*str)) str++;
        ptr = str;
        while (*ptr && !isspace(*ptr) && *ptr != '|') ptr++;
        // string: [str, ptr)
        std::string s(str, ptr - str);
        //printf("insert id=%d val=%s\n", tid, s.c_str());
        if (!dict.insert(std::make_pair(s, tid)).second)
            fail("duplicate key %s\n", s.c_str());
        str = ptr;
        while (*str && (isspace(*str) || *str == '|')) str++;
    } while (*ptr);
    tid++;
}

int MapSpecialTag::query(const std::string &str)
{
    std::map<std::string, int>::iterator it;
    it = dict.find(str);
    return it == dict.end() ? -1 : it->second;
}

int MapSpecialTag::query(const char *str) { return query(std::string(str)); }
int MapSpecialTag::query(const std::string &key, const std::string &val) { return query(key + '/' + val); }
int MapSpecialTag::query(const char *key, const char *val) { return query(std::string(key), std::string(val)); }

int MapSpecialTag::get_count() { return tid; }
