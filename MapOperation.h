#ifndef ZBY_MAPOPERATION_H
#define ZBY_MAPOPERATION_H
#include "common.h"
#include <vector>
#include <string>
#include "MapData.h"
#include "MapObject.h"
#include "MapGUI.h"

class MapGraphics;

class MapOperation {
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
        CLEAR_SELECT,
        CENTER_NUM_POINT, // center a previously numbered point
        CENTER_NUM_WAY,
        CENTER_SEL_POINT,
        CENTER_SEL_WAY,
        SHOW_WAYINFO,
        SHOW_NODEINFO,
        POP_DISPLAY,
        SHOW_QUERY_RESULT,
    };

    MapData *md;
    MapGraphics *mg;
    MapGUI *mgui;
    
    MapNode *snode; // selected node
    MapNode *nnode[MAX_KBDNUM]; // numbered node
    MapWay *sway; // selected way
    MapWay *nway[MAX_KBDNUM]; // numbered way
    
    // query
    std::vector<MapNode *> nresult;
    std::vector<MapWay *> wresult;
    std::wstring query_description;
    
    void query_name();
    void show_results();
    
    void select_way();
    void select_point();
    void number_way();
    void number_point();
    void clear_select();
    void show_wayinfo();
    void show_nodeinfo();
    
    void operation(MapOperationCode op);
};



#endif
