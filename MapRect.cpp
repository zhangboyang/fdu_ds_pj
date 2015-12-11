#include <cmath>
#include "common.h"
#include "MapRect.h"

#define RECT_CHECK(r) assert((r).left <= (r).right && (r).bottom <= (r).top);

#include <algorithm>
using namespace std;

MapRect::MapRect() {}
MapRect::MapRect(double left, double right, double bottom, double top)
    : left(left), right(right), bottom(bottom), top(top) { RECT_CHECK(*this) }
MapRect::MapRect(const MapRect &a, const MapRect &b)
{
    left = min(a.left, b.left);
    right = max(a.right, b.right);
    bottom = min(a.bottom, b.bottom);
    top = max(a.top, b.top);
}

bool MapRect::intersect(const MapRect &b) const { return intersect(*this, b); }

bool MapRect::intersect(const MapRect &a, const MapRect &b)
{
    RECT_CHECK(a) RECT_CHECK(b)
    bool ret = !(b.top < a.bottom || b.bottom > a.top || b.right < a.left || b.left > a.right);
    assert(ret == !(a.top < b.bottom || a.bottom > b.top || a.right < b.left || a.left > b.right));
    return ret;
}

void MapRect::merge(const MapRect &b)
{
    left = min(left, b.left);
    right = max(right, b.right);
    bottom = min(bottom, b.bottom);
    top = max(top, b.top);
}

double MapRect::max_distance() { RECT_CHECK(*this); return sqrt(sq(right - left) + sq(top - bottom)); }
double MapRect::area() { RECT_CHECK(*this); return (right - left) * (top - bottom); }

