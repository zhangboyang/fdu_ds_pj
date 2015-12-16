#ifndef ZBY_MAPGRAPHICS_H
#define ZBY_MAPGRAPHICS_H
#include "common.h"
#include "MapObject.h"
#include "MapData.h"
#include "MapGUI.h"

class MapGraphics {
    public:
    static const int NUM_MAX = 10;
    
    private:    
    enum MapGraphicsOperation {
        UP, DOWN, LEFT, RIGHT,
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
    
    std::vector<MapLine *> dll; // draw line list
    std::vector<MapWay *> dwl; // draw way list
    MapNode *snode; // selected node
    MapNode *nnode[NUM_MAX]; // numbered node
    //MapLine *sline; // selected line
    MapWay *sway; // selected way
    MapWay *nway[NUM_MAX]; // numbered way
    
    double mx, my; // mouse x, mouse y
    MapGraphicsOperation last_mouse_op;
    int kbd_num;
    
    double dminx, dmaxx, dminy, dmaxy;
    int window_width, window_height;
    std::vector<std::pair<std::pair<double, double>, int> > display_stack; // ((center_x, center_y), zoom_level)
    int zoom_level;
    int show_rtree;
    MapData *md;
    MapGUI *mgui;
    
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
    
    
    // all coord in MapGraphics is transformed by trans_gcoord()
    void trans_gcoord(double x, double y, double *gx, double *gy);
    void rtrans_gcoord(double gx, double gy, double *x, double *y);
    
    double get_display_resolution();
    int get_display_level_limit();
    void set_display_range(double dminx, double dmaxx, double dminy, double dmaxy);
    void push_display_range();
    void pop_display_range();
    void move_display_range(int x, int y);
    void zoom_display_range(int f);
    void zoom_display_by_size(double sizex, double sizey);
    void move_display_to_point(double gx, double gy);
    void reset_display_range();
    void center_way(MapWay *way);
    void center_point(MapNode *node);
    
    void map_operation(MapGraphicsOperation op);
    
    void set_mouse_coord(int x, int y);
    void mouse_event(bool use_last_op, int button, int state, int x, int y);
    
    void highlight_point(MapNode *node, float color[], float thick);
    void draw_way(MapWay *way);
    
    public:
    float scolor[3]; // selected color, R, G, B, from 0 to 1
    float ncolor[NUM_MAX][3]; // numbered color
    
    int selected_point_rect_size;
    float selected_point_rect_thick;
    float selected_way_thick;
    double zoom_bysize_factor;
    
    int initial_window_height;
    double move_step;
    double zoom_step;

    void target(MapData *md);
    void target_gui(MapGUI *mgui);
    void add_display_level(double res);
    
    void redraw();
    void reshape(int width, int height);
    void special_keyevent(int key, int x, int y);
    void keyevent(unsigned char key, int x, int y);
    void mouseupdown(int button, int state, int x, int y);
    void mousemotion(int x, int y);
    void show(const char *title, int argc, char *argv[]);
};
#endif
