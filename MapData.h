#ifndef ZBY_MAPDATA_H
#define ZBY_MAPDATA_H
#include "common.h"
#include "MapObject.h"
#include "MapRTree.h"
#include "MapLevel.h"
#include "MapWayType.h"
#include <vector>
#include <map>

class MapData {
    private:
    std::vector<MapNode *> nl; // l: object list
    std::vector<MapLine *> ll;
    std::vector<MapWay *> wl;
    std::vector<MapRelation *> rl;
    
    std::map<LL, MapNode *> nm; // m: map from id to object pointer
    std::map<LL, MapWay *> wm;
    std::map<LL, MapRelation *> rm;
    
    void trans_coord(double lat, double lon, double *x, double *y);
    void construct_line_by_signal_way(MapWay *way);
    void insert(MapLine *line);
    
    public:
    MapLevel ml;
    MapWayType wt;
    int dfactor; // display factor
    std::vector<MapRTree<MapLine *> > lrt;

    double minx, miny;
    double maxx, maxy;
    double minlat, minlon;
    double maxlat, maxlon;
    
    double map_ratio; // width / height (real)
    double geo_factor; // cos(lat)
    double max_geo_error;
    
    ~MapData();
    void insert(MapNode *node);
    void insert(MapWay *way);
    void insert(MapRelation *rela);
    void set_coord_limit(double minlat, double maxlat, double minlon, double maxlon);
    void set_node_coord_by_geo(MapNode *node, double lat, double lon);
    MapNode *get_node_by_id(LL id);
    void construct();
    void print_stat();
};

#endif
