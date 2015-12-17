#ifndef ZBY_MAPSPECIALTAG_H
#define ZBY_MAPSPECIALTAG_H
#include "common.h"
#include <map>
#include <string>

class MapSpecialTag {
    private:
    int tid;
    std::map<std::string, int> dict;
    public:
    MapSpecialTag();
    void insert(const char *str);
    int get_count();
    int query(const std::string &str);
    int query(const char *str);
    int query(const std::string &key, const std::string &val);
    int query(const char *key, const char *val);
};

#endif
