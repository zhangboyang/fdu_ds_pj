#ifndef ZBY_MAPDATA_H
#define ZBY_MAPDATA_H
#include "common.h"
#include "MapObject.h"
#include "MapRTree.h"
#include "MapLevel.h"
#include "MapWayType.h"
#include "MapDict.h"
#include <vector>
#include <map>

const double dist_factor = 4.0009e7 / 360; // from lat to meters

class MapData {
    private:
    std::vector<MapNode *> nl; // l: object list
    std::vector<std::vector<MapLine *> > ll; // line list by level
    std::vector<MapWay *> wl;
    std::vector<MapRelation *> rl;
    
    std::map<LL, MapNode *> nm; // m: map from id to object pointer
    std::map<LL, MapWay *> wm;
    std::map<LL, MapRelation *> rm;
    
    void trans_coord(double lat, double lon, double *x, double *y);
    
    int tot_lvl;
    
    public:
    MapDict<MapWay *> wd; // dict of ways
    MapDict<MapNode *> nd; // dict of nodes
    
    MapLevel ml;
    MapWayType wt;
    MapRTree<MapLine *> *lrt;

    double dfactor; // display factor
    double line_detail_dist_low_limit_factor;
    double line_detail_dist_high_limit_factor;
    double line_detail_angle_limit;
    
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
