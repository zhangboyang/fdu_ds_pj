#ifndef ZBY_MAPOBJECT_H
#define ZBY_MAPOBJECT_H
#include "common.h"
#include "MapRect.h"

#include <string>
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
    
    LL id;
    std::vector<std::pair<std::string, std::wstring> > tl; // tl: tag list (key->value)

    void set_id(LL id);
};

class MapNode : public MapObject {
    public:
    double x, y;
    double lat, lon;

    MapRect get_rect();
};

class MapWay : public MapObject {
    public:
    int waytype;
    int level;
    MapRect rect;
    
    std::vector<std::vector<MapNode *> > nl; // nl: node list
    void add_node(MapNode *node);
    MapRect get_rect();
    MapWay();
};

class MapLine : public MapObject {
    public:
    MapNode *p1, *p2; // p1 -> p2
    MapLine *prev, *next; // pointer to prev and next edge in way FIXME
    MapWay *way; // pointer to way
    
    void set_line(MapNode *p1, MapNode *p2);
    void set_way(MapWay *way);
    MapRect get_rect();
};

class MapRelation : public MapObject {
    public:
    // FIXME: 'relation' not implemented
};

#endif
