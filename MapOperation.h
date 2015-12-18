#ifndef ZBY_MAPOPERATION_H
#define ZBY_MAPOPERATION_H
#include "common.h"
#include <ctime>
#include <vector>
#include <string>
#include "MapData.h"
#include "MapObject.h"
#include "MapGUI.h"
#include "MapVector.h"

class MapGraphics;

class MapOperation {
    private:
    double qtime; // query time
    clock_t qclock; // start clock
    double nearby_distsq;
    MapPoint nearby_center;
    
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
    
    void query_name();
    void show_results();
    void select_way();
    void select_point();
    void number_way();
    void number_point();
    void show_wayinfo();
    void show_nodeinfo();
    void query_tag_with_poly();
    void query_tag_with_dist();
    void query_tag_in_display();
    void query_tag();
    
    void query_timer_start();
    void query_timer_stop();
    
    public:
    static const int MAX_KBDNUM = 10;
    
    enum MapOperationCode {
        MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT,
        ZOOM_OUT, ZOOM_IN,
        RESET_VIEW,
        TOGGLE_RTREE,
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
    };

    MapData *md;
    MapGraphics *mg;
    MapGUI *mgui;
    
    MapNode *snode; // selected node
    MapNode *nnode[MAX_KBDNUM]; // numbered node
    MapWay *sway; // selected way
    MapWay *nway[MAX_KBDNUM]; // numbered way
    
    std::vector<MapPoint> pvl; // poly vertex list
    
    // query
    std::vector<MapNode *> nresult;
    std::vector<MapWay *> wresult;
    std::wstring query_description;
    void clear_all();
    void operation(MapOperationCode op);
};



#endif
