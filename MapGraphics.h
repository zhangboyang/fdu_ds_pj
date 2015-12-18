#ifndef ZBY_MAPGRAPHICS_H
#define ZBY_MAPGRAPHICS_H
#include "common.h"
#include "MapObject.h"
#include "MapData.h"
#include "MapGUI.h"
#include "MapOperation.h"

class MapGraphics {
    private:
    MapOperation::MapOperationCode last_mouse_op;
    
    double dminx, dmaxx, dminy, dmaxy;
    int window_width, window_height;
    std::vector<std::pair<std::pair<double, double>, int> > display_stack; // ((center_x, center_y), zoom_level)
    int zoom_level;
    
    MapOperation *mo;
    MapData *md;
    MapGUI *mgui;
    
    double get_display_resolution();
    void update_current_display_level();
    int get_display_level_limit();
    void set_display_range(double dminx, double dmaxx, double dminy, double dmaxy);
    
    void map_operation(MapOperation::MapOperationCode op);
    
    void set_mouse_coord(int x, int y);
    void mouse_event(bool use_last_op, int button, int state, int x, int y);
    
    void print_string(const char *str);
    void highlight_point(MapNode *node, double size, float color[], float thick);
    void draw_way(MapWay *way);
    void draw_vertex(double x, double y);

    std::vector<double> refresh_time; // last redraw time, in ms
    double last_operation_time;
    int vertex_count;
    std::string last_redraw_str;
        
    public:
    std::string msg;
    
    std::vector<MapLine *> dll; // draw line list
    std::vector<MapWay *> dwl; // draw way list
    
    int clvl; // current display level
    double mx, my; // mouse x, mouse y
    int kbd_num;
    int show_rtree;
    
    float pcolor[3]; // poly color
    float pthickness;
    float scolor[3]; // selected color, R, G, B, from 0 to 1
    float ncolor[MapOperation::MAX_KBDNUM][3]; // numbered color
    float nrcolor[3]; // node result color
    float nrthick;
    double nrsize;
    
    int selected_point_rect_size;
    float selected_point_rect_thick;
    float selected_way_thick;
    double zoom_bysize_factor;
    
    int initial_window_height;
    double move_step;
    double zoom_step;

    void target(MapData *md);
    void target_gui(MapGUI *mgui);
    void target_operation(MapOperation *mo);

    
    void get_display_range(double *minx, double *maxx, double *miny, double *maxy);
    void push_display_range();
    void pop_display_range();
    void move_display_range(int x, int y);
    void zoom_display_range(int f);
    void zoom_display_by_size(double sizex, double sizey);
    void move_display_to_point(double gx, double gy);
    void reset_display_range();
    void center_way(MapWay *way);
    void center_point(MapNode *node);
        
    void redraw();
    void reshape(int width, int height);
    void special_keyevent(int key, int x, int y);
    void keyevent(unsigned char key, int x, int y);
    void mouseupdown(int button, int state, int x, int y);
    void mousemotion(int x, int y);
    void show(const char *title, int argc, char *argv[]);
};
#endif
