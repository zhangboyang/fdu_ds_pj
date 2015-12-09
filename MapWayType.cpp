#include <cstdio>
#include "common.h"
#include "MapWayType.h"

void MapWayType::insert(const char *str)
{
    // way type defination: name|level|r|g|b|thick
    waytype_t t;
    int ret = sscanf(str, "%*[^| ] | %d | %f | %f | %f | %f", &t.level, &t.r, &t.g, &t.b, &t.thick);
    if (ret != 5) fail("can't parse '%s' to waytype, ret = %d", str, ret);
    int id = data.size();
    data.push_back(t);
    const char *ptr = str;
    while (*ptr && *ptr != '|' && !isspace(*ptr)) ptr++;
    std::string key(str, ptr - str);
    assert(name2id.find(key) == name2id.end());
    name2id.insert(make_pair(key, id));
}

int MapWayType::query_id(const char *name)
{
    assert(data.size() > 0);
    std::map<std::string, int>::iterator it;
    it = name2id.find(std::string(name));
    if (it == name2id.end()) return 0; // return default type 0 when not found
    return it->second;
}

int MapWayType::query_level(int id) { assert(data.size() > 0); return data[id].level; }
void MapWayType::query_rgb(int id, float *r, float *g, float *b) { assert(data.size() > 0); *r = data[id].r; *g = data[id].g; *b = data[id].b; }
float MapWayType::query_thick(int id) { assert(data.size() > 0); return data[id].thick; }

