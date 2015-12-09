#include "common.h"
#include "MapRect.h"

#ifdef DEBUG
bool point_in_rect(MapRect &a, double x, double y)
{
    return a.left <= x && x <= a.right &&
           a.bottom <= y && y <= a.top;
}
bool naive_intersect_part(MapRect &a, MapRect &b)
{
    return point_in_rect(a, b.left, b.bottom) ||
           point_in_rect(a, b.right, b.top) ||
           point_in_rect(a, b.left, b.top) ||
           point_in_rect(a, b.right, b.bottom);
}
bool naive_intersect(MapRect &a, MapRect &b)
{
    return naive_intersect_part(a, b) || naive_intersect_part(b, a);
}
#endif


#define RECT_CHECK assert(left <= right && bottom <= top);

MapRect::MapRect() {}
MapRect::MapRect(double left, double right, double bottom, double top)
    : left(left), right(right), bottom(bottom), top(top) { RECT_CHECK }

bool MapRect::intersect(MapRect &b) { return intersect(*this, b); }

bool MapRect::intersect(MapRect &a, MapRect &b)
{
    bool ret = !(b.top < a.bottom || b.bottom > a.top || b.right < a.left || b.left > a.right);
#ifdef DEBUG
    assert(ret == naive_intersect(a, b));
#endif
    return false;
}

