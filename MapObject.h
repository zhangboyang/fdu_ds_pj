#ifndef ZBY_MAPOBJECT_H
#define ZBY_MAPOBJECT_H
#include "common.h"

#include <vector>
#include <map>

/*
    +MapObject
        +MapNode
        +MapLine
        +MapWay
        +MapRelation
*/

class MapObject {
    public:
    enum ObjectType {
        NONE, NODE, LINE, WAY, RELATION,
    };
    
    LL id;

    void set_id(LL id);
    //virtual ObjectType type();
    //virtual ~MapObject();
};

class MapNode : public MapObject {
    public:
    double x, y;
    //double lat, lon;
    
    //virtual ObjectType type();
};

class MapLine : public MapObject {
    public:
    //virtual ObjectType type();
    //double x1, y1;
    //double x2, y2;
    MapNode *p1, *p2;
    void set_line(MapNode *p1, MapNode *p2);
};

class MapWay : public MapObject {
    public:
    //virtual ObjectType type();
    std::vector<MapNode *> nl; // nl: node list
    void add_node(MapNode *node);
};

class MapRelation : public MapObject {
    public:
    //virtual ObjectType type();
    // FIXME: 'relation' not implemented
};

#endif
