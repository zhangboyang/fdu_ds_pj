#include <cmath>
#include "common.h"
#include "MapRect.h"

#define RECT_CHECK(r) assert((r).left <= (r).right && (r).bottom <= (r).top);

MapRect::MapRect() {}
MapRect::MapRect(double left, double right, double bottom, double top)
    : left(left), right(right), bottom(bottom), top(top) { RECT_CHECK(*this) }

bool MapRect::intersect(const MapRect &b) const { return intersect(*this, b); }

bool MapRect::intersect(const MapRect &a, const MapRect &b)
{
    RECT_CHECK(a) RECT_CHECK(b)
    bool ret = !(b.top < a.bottom || b.bottom > a.top || b.right < a.left || b.left > a.right);
    return ret;
}

double MapRect::max_distance() { RECT_CHECK(*this); return sqrt(sq(right - left) + sq(top - bottom)); }

