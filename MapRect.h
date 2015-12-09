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
  +-----x
*/


class MapRect {
    public:
    double left, right, bottom, top;
    
    MapRect();
    MapRect(double left, double right, double bottom, double top);

    static bool intersect(MapRect &a, MapRect &b);
    bool intersect(MapRect &b);

};

#endif
