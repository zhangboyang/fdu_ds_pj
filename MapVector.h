#ifndef ZBY_MAPVECTOR_H
#define ZBY_MAPVECTOR_H
#include "common.h"
#include <cmath>

struct MapVector {
    double x, y;
    MapVector() {}
    MapVector(double x, double y) { MapVector::x = x; MapVector::y = y; }
    MapVector operator - (const MapVector &b) const { return MapVector(x - b.x, y - b.y); }
    MapVector operator + (const MapVector &b) const { return MapVector(x + b.x, y + b.y); }
};

typedef MapVector MapPoint;



inline static double det(const MapVector &a, const MapVector &b) { return a.x * b.y - a.y * b.x; }
inline static double dot(const MapVector &a, const MapVector &b) { return a.x * b.x + a.y * b.y; }
inline static double lensq(const MapVector &a) { return dot(a, a); } // sq means squared
inline static double len(const MapVector &a) { return sqrt(lensq(a)); }
inline static double angle(const MapVector &a, const MapVector &b) { return acos(dot(a, b) / len(a) / len(b)); }

inline static double distsq_p2s(const MapPoint &p, const MapPoint &a, const MapPoint &b)
{
    MapVector v1 = b - a, v2 = p - a, v3 = p - b;
    if (dot(v1, v2) < 0) return lensq(v2);
    else if (dot(v1, v3) > 0) return lensq(v3);
    else return sq(det(v1, v2)) / lensq(v1);
}

inline static double dist_p2s(const MapPoint &p, const MapPoint &a, const MapPoint &b) { return sqrt(distsq_p2s(p, a, b)); }

#endif
