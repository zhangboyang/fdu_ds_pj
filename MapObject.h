#ifndef ZBY_MAPOBJECT_H
#define ZBY_MAPOBJECT_H
#include "common.h"
#include "MapRect.h"
//#include "MapVector.h"

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
    double lat, lon;
    
    std::map<std::string, const wchar_t *> names;
    //MapPoint get_point();
    //virtual ObjectType type();
    ~MapNode();
};

class MapWay : public MapObject {
    public:
    //virtual ObjectType type();
    //int level;
    int waytype;
    std::vector<MapNode *> nl; // nl: node list
    
    std::map<std::string, const wchar_t *> names;
    //static bool compare_by_waytype(MapWay *a, MapWay *b); // useful when sorting ways
    void add_node(MapNode *node);
    //void set_level(int level);
    MapRect get_rect();
    //void get_center(double *x, double *y);
    //void get_xysize(double *sizex, double *sizey);
    ~MapWay();
};

class MapLine : public MapObject {
    public:
    //virtual ObjectType type();
    //double x1, y1;
    //double x2, y2;
    MapNode *p1, *p2; // p1 -> p2
    MapLine *prev, *next; // pointer to prev and next edge in way
    MapWay *way; // pointer to way
    void set_line(MapNode *p1, MapNode *p2);
    void set_way(MapWay *way);
    MapRect get_rect();
};

class MapRelation : public MapObject {
    public:
    //virtual ObjectType type();
    // FIXME: 'relation' not implemented
};

#endif
