#ifndef ZBY_MAPRECT_H
#define ZBY_MAPRECT_H
#include "common.h"


/*
                   top      (right, top)
             +--------------+
             |              |
         left|              |right
  y          |              |
  |          +--------------+
  |  (left, bottom) bottom
  |
  +------x
*/


class MapRect {
    public:
    double left, right, bottom, top;
    
    MapRect();
    MapRect(double left, double right, double bottom, double top);
    MapRect(const MapRect &a, const MapRect &b); // construct by merging two rectanges

    static bool intersect(const MapRect &a, const MapRect &b);
    bool intersect(const MapRect &b) const;
    void merge(const MapRect &b);
    double max_distance();
    double area();
};

#endif
