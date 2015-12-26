#ifndef ZBY_MAPOPERATION_H
#define ZBY_MAPOPERATION_H
#include "common.h"
#include <vector>
#include <string>
#include "MapData.h"
#include "MapObject.h"
#include "MapGUI.h"
#include "MapVector.h"
#include "MapShortestPath.h"
#include "MapTaxiRoute.h"

class MapGraphics;

class MapOperation {
    private:
    double qtime; // query time
    double qclock; // start clock
    std::wstring query_description;
    double nearby_distsq;
    static MapPoint nearby_center;
    static bool sort_node_by_dist(MapNode *a, MapNode *b);
    static bool sort_way_by_dist(MapWay *a, MapWay *b);
    
    void query_tag_with_filter(const std::string &tag, const MapRect &baserect,
          bool (MapOperation::*node_filter)(MapNode *),
          bool (MapOperation::*way_filter)(MapWay *));
    bool poly_node_filter(MapNode *node);
    bool poly_way_filter(MapWay *way);
    bool dist_node_filter(MapNode *node);
    bool dist_way_filter(MapWay *way);
    bool true_node_filter(MapNode *node); // always return true
    bool true_way_filter(MapWay *way);
    
    void add_polyvertex();
    void clear_polyvertex();
    void select_results();
    void clear_results();
    void clear_select();
    
    MapNode *choose_nearest_sp_node();
    void set_shortestpath_start();
    void set_shortestpath_end();
    void run_shortestpath();
    void show_shortestpath_result();
    void clear_shortestpath_vertex();
    void clear_shortestpath_result();
    void switch_shortest_algo();
    
    void select_taxi_route();
    void show_taxi_list();
    void clear_taxi_route();
    void reload_taxi_route();
    void show_taxi_route_begin();
    void show_taxi_route_end();
    void generate_taxi_route_node_report();
    void switch_taxi_route_node(int f);
    
    void query_name();
    void select_way();
    void select_point();
    void number_way();
    void number_point();
    void show_results();
    void msgbox_append_tags(MapObject *ptr);
    void msgbox_append_names(MapObject *ptr);
    void show_wayinfo();
    void show_nodeinfo();
    void query_tag_with_poly();
    void query_tag_with_dist();
    void query_tag_in_display();
    void query_tag();
    
    bool query_main_name(MapObject *ptr, std::wstring &name);
    std::wstring get_node_string(MapNode *node);
    std::wstring get_way_string(MapWay *way);
    
    void query_timer_start();
    void query_timer_stop();
    
    public:
    static const int MAX_KBDNUM = 10;
    
    enum MapOperationCode {
        NOP,
        MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT,
        ZOOM_OUT, ZOOM_IN, ZOOM_OUT_BY_MOUSE, ZOOM_IN_BY_MOUSE,
        RESET_VIEW,
        QUERY_NAME,
        SELECT_WAY,
        SELECT_POINT,
        NUMBER_POINT, // assign a number to currently selected point
        NUMBER_WAY,
        CLEAR_ALL,
        CENTER_NUM_POINT, // center a previously numbered point
        CENTER_NUM_WAY,
        CENTER_SEL_POINT,
        CENTER_SEL_WAY,
        SHOW_WAYINFO,
        SHOW_NODEINFO,
        POP_DISPLAY,
        SHOW_QUERY_RESULT,
        ADD_POLYVERTEX,
        CLEAR_POLYVERTEX,
        QUERY_TAG,
        SET_SHORTESTPATH_START,
        SET_SHORTESTPATH_END,
        RUN_SHORTESTPATH,
        SHOW_SHORTESTPATH_RESULT,
        CLEAR_SHORTESTPATH,
        SWITCH_SHORTESTPATH_ALGO,
        SELECT_TAXI_ROUTE,
        SHOW_TAXI_LIST,
        SHOW_TAXI_ROUTE_BEGIN,
        SHOW_TAXI_ROUTE_END,
        SHOW_TAXI_ROUTE_NEXT_NODE,
        SHOW_TAXI_ROUTE_PREV_NODE,
    };

    MapData *md;
    MapGraphics *mg;
    MapGUI *mgui;
    MapShortestPath *msp;
    MapTaxiRoute *mtr;
    
    MapNode *snode; // selected node
    MapNode *nnode[MAX_KBDNUM]; // numbered node
    MapWay *sway; // selected way
    MapWay *nway[MAX_KBDNUM]; // numbered way
    
    MapNode *sp_start, *sp_end; // shortest-path start/end node
    std::vector<MapLine *> sp_result;
    double sp_mindist, sp_time, sp_prepare_time;
    int sp_algo;
    std::string sp_report; // report is displayed by bitmap string
    std::string query_report;
    std::string taxi_report, taxi_node_report;
    
    std::vector<MapPoint> pvl; // poly vertex list
    
    // query
    std::vector<MapNode *> nresult; // node result
    std::vector<MapWay *> wresult;
    
    // taxi
    std::vector<std::pair<MapPoint, MapTaxiRoute::taxi_node *> > tr; // taxi route
    int cur_tr_node;
    
    void clear_all();
    void init();
    void operation(MapOperationCode op);
};



#endif
