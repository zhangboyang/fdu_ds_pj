#ifndef ZBY_WAYTYPE_H
#define ZBY_WAYTYPE_H
#include "common.h"

#include <map>
#include <vector>
#include <string>

class MapWayType {
    private:
    struct waytype_t { int level; float r, g, b; float thick; };
    std::map<std::string, int> name2id;
    std::vector<waytype_t> data;
    public:
    void insert(const char *str);
    int query_id(const std::string &name);
    int query_level(int id);
    void query_rgb(int id, float *r, float *g, float *b);
    float query_thick(int id);
};

#endif
