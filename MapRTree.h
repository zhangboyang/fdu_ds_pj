#ifndef ZBY_MAPRTREE_H
#define ZBY_MAPRTREE_H
#include "common.h"
#include "MapRect.h"

#include <vector>

template <class TP>
class MapRTree {
    private:
    std::vector<TP> a;
    public:
    void insert(TP obj);
    void find(std::vector<TP> &result, const MapRect &rect);
};

template <class TP>
void MapRTree<TP>::insert(TP obj) { a.push_back(obj); }

template <class TP>
void MapRTree<TP>::find(std::vector<TP> &result, const MapRect &rect)
{
    result.clear();
    typename std::vector<TP>::iterator it;
    for (it = a.begin(); it != a.end(); it++) {
        if (rect.intersect((*it)->get_rect()))
            result.push_back(*it);
    }
}

#endif
