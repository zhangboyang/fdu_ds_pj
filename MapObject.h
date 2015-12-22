#ifndef ZBY_MAPOBJECT_H
#define ZBY_MAPOBJECT_H
#include "common.h"
#include "MapRect.h"

#include <string>
#include <vector>
#include <map>

class MapObject;
class MapNode;
class MapLine;
class MapWay;
class MapRelation;

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
    
    double dist; // distance for shortest path
    std::vector<MapLine *> edges; // edges for shortest path algorithm
    MapLine *from; // from which egde to go to this node
    int flag; // flag var by shortest path
    bool on_shortest_path; // is this node connect to a way with 'on_shortest_path == true'
    
    MapNode();
    MapRect get_rect();
};

class MapWay : public MapObject {
    public:
    int waytype;
    int level;
    MapRect rect;
    
    bool on_shortest_path; // is the way might be the edge on shortest path
    bool one_way;
    
    std::vector<std::vector<MapNode *> > nl; // nl: node list
    void add_node(MapNode *node);
    MapRect get_rect();
    double calc_length(); // result NOT multiplyed by dist_factor
    double calc_area(); // result NOT multiplyed by sq(dist_factor)
    bool is_closed(); // close means loop
    MapWay();
};

class MapLine : public MapObject {
    public:
    MapNode *p1, *p2; // p1 -> p2
    MapWay *way; // pointer to way
    double len; // edge length, only valid in level 0
    
    void set_line(MapNode *p1, MapNode *p2);
    void set_way(MapWay *way);
    MapRect get_rect();
};

class MapRelation : public MapObject {
    public:
    // FIXME: 'relation' not implemented
};

#endif
