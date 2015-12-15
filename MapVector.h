#ifndef ZBY_MAPVECTOR_H
#define ZBY_MAPVECTOR_H
#include "common.h"


struct MapVector {
    double x, y;
    MapVector();
    MapVector(double x, double y);
};

typedef MapVector MapPoint;

MapVector operator - (const MapVector &a, const MapVector &b);
MapVector operator + (const MapVector &a, const MapVector &b);
double det(const MapVector &a, const MapVector &b);
double dot(const MapVector &a, const MapVector &b);
double len(const MapVector &a);

double dts(const MapPoint &p, const MapPoint &a, const MapPoint &b); // distance to segment (from point)

#endif
