#ifndef ZBY_MAPGRAPHICS_H
#define ZBY_MAPGRAPHICS_H
#include "common.h"
#include "MapObject.h"
#include "MapData.h"
#include "MapGUI.h"

class MapGraphics {
    private:
    enum MapGraphicsOperation {
        UP, DOWN, LEFT, RIGHT,
        ZOOM_OUT, ZOOM_IN,
        RESET_VIEW,
        TOGGLE_RTREE,
        QUERY_NAME,
        SELECT_WAY,
        SELECT_POINT,
    };
    
    std::vector<MapLine *> dll; // draw line list
    std::vector<MapWay *> dwl; // draw way list
    MapNode *snode; // selected node
    //MapLine *sline; // selected line
    MapWay *sway; // selected way
    
    double mx, my; // mouse x, mouse y
    MapGraphicsOperation last_mouse_op;
    
    double dminx, dmaxx, dminy, dmaxy;
    int window_width, window_height;
    int zoom_level;
    int show_rtree;
    MapData *md;
    MapGUI *mgui;
    
    
    void select_way();
    void select_point();
    void query_name();
    
    // all coord in MapGraphics is transformed by trans_gcoord()
    void trans_gcoord(double x, double y, double *gx, double *gy);
    void rtrans_gcoord(double gx, double gy, double *x, double *y);
    
    double get_display_resolution();
    int get_display_level_limit();
    void set_display_range(double dminx, double dmaxx, double dminy, double dmaxy);
    void move_display_range(int x, int y);
    void zoom_display_range(int f);
    void reset_display_range();
    void map_operation(MapGraphicsOperation op);
    
    void set_mouse_coord(int x, int y);
    void mouse_event(bool use_last_op, int button, int state, int x, int y);
    
    public:
    float scolor[3]; // selected color, R, G, B, from 0 to 1
    int selected_point_rect_size;
    float selected_point_rect_thick;
    float selected_way_thick;
        
    int initial_window_height;
    double move_step;
    double zoom_step;

    void target(MapData *md);
    void target_gui(MapGUI *mgui);
    void add_display_level(double res);
    
    void redraw();
    void reshape(int width, int height);
    void special_keyevent(int key, int x, int y);
    void mouseupdown(int button, int state, int x, int y);
    void mousemotion(int x, int y);
    void show(const char *title, int argc, char *argv[]);
};
#endif
