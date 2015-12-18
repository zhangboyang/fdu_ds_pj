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
    std::vector<wchar_t *> lwstr_list; // allocated memory pointers
    wchar_t to_lower(wchar_t wc)
    {
        static wchar_t upper[] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        static wchar_t lower[] = L"abcdefghijklmnopqrstuvwxyz";
        wchar_t *p;
        if (wc && (p = wcschr(upper, wc))) return lower[p - upper];
        return wc;
    }
    void wstr_tolower(wchar_t *wstr) { while (*wstr) *wstr = to_lower(*wstr), wstr++; }
    public:
    ~MapDict() { for (std::vector<wchar_t *>::iterator it = lwstr_list.begin(); it != lwstr_list.end(); it++) free(*it); }
    void insert(const wchar_t *key, TP value) {
        wchar_t *lkey = wcsdup(key);
        wstr_tolower(lkey);
        lwstr_list.push_back(lkey);
        while (*lkey) dict.push_back(kvpair(lkey++, value));
    }
    void construct() { std::sort(dict.begin(), dict.end()); }
    void find(std::vector<TP> &result, const wchar_t *wstr, int limit)
    {
        wchar_t *lwstr = wcsdup(wstr);
        wstr_tolower(lwstr);
        typename std::vector<kvpair>::iterator it;
        it = std::lower_bound(dict.begin(), dict.end(), kvpair(lwstr, NULL));
        int cnt = 0;
        int begin = result.size();
        while (it != dict.end()) {
            if (!it->is_prefix(lwstr)) break;
            if (limit >= 0 && ++cnt > limit) break;
            result.push_back(it->value);
            it++;
        }
        std::sort(result.begin() + begin, result.end());
        result.resize(std::unique(result.begin() + begin, result.end()) - (result.begin() + begin));
        free(lwstr);
    }
    void find(std::vector<TP> &result, const wchar_t *wstr) { find(result, wstr, -1); }
};
#endif
