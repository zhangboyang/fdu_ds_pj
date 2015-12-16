#ifndef ZBY_MAPDICT_H
#define ZBY_MAPDICT_H
#include "common.h"
#include <cwchar>
#include <vector>
#include <algorithm>

template <class TP>
class MapDict {
    private:
    class kvpair {
        public:
        const wchar_t *key;
        TP value;
        kvpair(const wchar_t *key, TP value) { kvpair::key = key; kvpair::value = value; }
        bool operator < (const kvpair &b) const { return wcscmp(key, b.key) < 0; }
        bool is_prefix(const wchar_t *pre) const { return wcsncmp(key, pre, wcslen(pre)) == 0; }
    };
    std::vector<kvpair> dict;
    public:
    void insert(const wchar_t *key, TP value) { while (*key) dict.push_back(kvpair(key++, value)); }
    void construct() { std::sort(dict.begin(), dict.end()); }
    void find(std::vector<TP> &result, const wchar_t *wstr, int limit)
    {
        typename std::vector<kvpair>::iterator it;
        it = std::lower_bound(dict.begin(), dict.end(), kvpair(wstr, NULL));
        int cnt = 0;
        while (it != dict.end()) {
            if (!it->is_prefix(wstr)) break;
            if (limit >= 0 && ++cnt > limit) break;
            result.push_back(it->value);
            it++;
        }
    }
    void find(std::vector<TP> &result, const wchar_t *wstr) { find(result, wstr, -1); }
};
#endif
