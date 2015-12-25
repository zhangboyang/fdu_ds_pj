#ifndef ZBY_MAPTAXIROUTE_H
#define ZBY_MAPTAXIROUTE_H
#include "common.h"
#include <vector>

class MapTaxiRoute {
    const char *fn;
    public:
    struct taxi_node {
        int taxi_id;
        char timestr[20];
        int is_alert;
        double lon, lat;
        int is_empty;
        char light_state;
        int is_highway;
        int is_brake;
        float speed;
        float direction;
        int gps_count;
    };
    
    std::vector<taxi_node> tnl; // taxi node list
    std::vector<std::pair<int, std::pair<int, long> > > tl; // taxi list (id, (node count, offset))
    
    void set_filename(const char *fn); // fn shouldn't be temporary storage
    void preprocess();
    int find_index_by_id(int taxi_id);
    void load_route(int taxi_index);
};
#endif
