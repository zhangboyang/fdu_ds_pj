#ifndef ZBY_MAPOBJECT_H
#define ZBY_MAPOBJECT_H
#include "common.h"

#include <vector>
#include <map>

/*
    +MapObject
        +MapNode
        +MapWay
        +MapRelation
*/

class MapObject {
    public:
    LL id;

    void set_id(LL id);
};

class MapNode : public MapObject {
    public:
    static double gminx, gmaxx, gminy, gmaxy;
    double x, y;
    double lat, lon;
    
    static void trans_coord(double lat, double lon, double *x, double *y);
    void set_coord(double lat, double lon);
    static void set_glimit(double gminx, double gmaxx, double gminy, double gmaxy);
    void get_gcoord(double *gx, double *gy);
};

class MapWay : public MapObject {
    public:
    std::vector<MapNode *> nl; // nl: node list
    void add_node(MapNode *node);
};

class MapRelation : public MapObject {
    public:
    // FIXME
};

#endif
