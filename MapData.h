#ifndef ZBY_MAPDATA_H
#define ZBY_MAPDATA_H
#include "common.h"
#include "MapObject.h"

#include <vector>
#include <map>

class MapData {
    public:
    std::vector<MapNode *> nl; // l: object list
    std::vector<MapWay *> wl;
    std::vector<MapRelation *> rl;
    
    std::map<LL, MapNode *> nm; // m: map from id to object pointer
    std::map<LL, MapWay *> wm;
    std::map<LL, MapRelation *> rm;
    
    double minlat, minlon;
    double maxlat, maxlon;
    double minx, miny;
    double maxx, maxy;
    double map_ratio; // width / height
    double geo_factor; // cos(lat)
    
    ~MapData();
    void insert(MapNode *node);
    void insert(MapWay *way);
    void insert(MapRelation *rela);
    void set_coord_limit(double minlat, double maxlat, double minlon, double maxlon);
    void set_node_coord_by_geo(MapNode *node, double lat, double lon);
    void trans_coord(double lat, double lon, double *x, double *y);
    MapNode *get_node_by_id(LL id);
    void print_stat();
};

#endif
