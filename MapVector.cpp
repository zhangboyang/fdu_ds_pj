#include <cmath>
#include "common.h"
#include "MapVector.h"

#define D double
#define V MapVector
#define P MapPoint
#define C const
#define O operator
#define R return

V::V() {}
V::V(D x, D y) { V::x = x; V::y = y; }

V O - (C V &a, C V &b) { R V(a.x - b.x, a.y - b.y); }
V O + (C V &a, C V &b) { R V(a.x + b.x, a.y + b.y); }
D det(C V &a, C V &b) { R a.x * b.y - a.y * b.x; }
D dot(C V &a, C V &b) { R a.x * b.x + a.y * b.y; }
D len(C V &a) { R sqrt(dot(a, a)); }

D dts(C P &p, C P &a, C P &b)
{
    V v1 = b - a, v2 = p - a, v3 = p - b;
    if (dot(v1, v2) < 0) return len(v2);
    else if (dot(v1, v3) > 0) return len(v3);
    else return fabs(det(v1, v2)) / len(v1);
}
