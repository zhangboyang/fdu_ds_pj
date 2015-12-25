#ifndef ZBY_MAPGRAPHICS_H
#define ZBY_MAPGRAPHICS_H
#include "common.h"
#include "MapObject.h"
#include "MapData.h"
#include "MapGUI.h"
#include "MapOperation.h"
#include "MapShortestPath.h"
#include "MapTaxiRoute.h"

class MapGraphics {
    private:
    MapOperation::MapOperationCode last_mouse_op;
    
    double rtminx, rtmaxx, rtminy, rtmaxy;
    double dminx, dmaxx, dminy, dmaxy;
    bool is_dragging_map;
    int window_width, window_height;
    std::vector<std::pair<std::pair<double, double>, int> > display_stack; // ((center_x, center_y), zoom_level)
    int zoom_level;
    double dwl_reload_time, dll_reload_time;

    std::vector<double> refresh_time; // last redraw time, in ms
    double last_operation_time;
    int vertex_count;
    std::string last_redraw_str;
    
    MapOperation *mo;
    MapData *md;
    MapGUI *mgui;
    MapShortestPath *msp;
    MapTaxiRoute *mtr;
    
    struct vnode {
        float x, y;
        float r, g, b;
    };
    
    std::vector<unsigned> ibuffer, vbuffer; // gl buffers
    std::vector<std::map<float, std::pair<std::vector<vnode>, std::vector<unsigned> > > > vct_tvil; // thickness map
    std::vector<std::vector<vnode> > vct_vl; // vertex list
    std::vector<std::vector<unsigned> > vct_il; // indice list
    std::vector<std::vector<std::pair<float, std::pair<int, int> > > > vct_tl; // <thickness, <offset, size> >, used when drawing
    
    void trans_gcoord(double x, double y, double *gx, double *gy);
    double get_display_resolution();
    void update_current_display_level();
    int get_display_level_limit();
    void set_display_range(double dminx, double dmaxx, double dminy, double dmaxy);
    
    void map_operation(MapOperation::MapOperationCode op);
    
    void set_kbd_specialkey();
    
    void set_mouse_coord(int x, int y);
    void mouse_event(bool use_last_op, int button, int state, int x, int y);
    
    void print_string(const char *str);
    void highlight_point(MapNode *node, double size, float color[], float thick);
    void draw_way(MapWay *way, bool force_level = false);
    void draw_vertex(double x, double y);
    void reload_vertex();
    void reload_dll(double minx, double maxx, double miny, double maxy); // query r-tree
    void reload_dwl(); // reload dwl
    void load_current_level_buffer();
    void show_loading_screen();
        
    public:
    std::string msg;
    
    std::vector<MapLine *> dll; // draw line list, can be relied, every redraw reloaded
    std::vector<MapWay *> dwl; // draw way list, shouldn't be relied, since it's not update when no-rtree drawing
    
    int clvl, last_clvl; // current display level
    int tlvl; // total display level
    double mx, my; // mouse x, mouse y
    double last_mx, last_my;
    int kbd_num;
    int kbd_shift;
    int show_rtree;
    int show_shortestpath_node;
    int show_message;
    int select_flag;
    
    float pcolor[3]; // poly color
    float pthickness;
    float scolor[3]; // selected color, R, G, B, from 0 to 1
    float ncolor[MapOperation::MAX_KBDNUM][3]; // numbered color
    float nrcolor[3]; // node result color
    float nrthick;
    float wrcolor[3]; // way result color
    float wrthick;
    float trcolor[3]; // taxi route color
    float trthickness;
    double nrsize;
    
    float sp_src_color[3], sp_dest_color[3], sp_path_color[3]; // shortest path color
    float sp_path_thick;
    double sp_vertex_rect_size;
    float sp_vertex_rect_thick;
    
    double selected_point_rect_size;
    float selected_point_rect_thick;
    float selected_way_thick;
    double zoom_bysize_factor;
    
    int initial_window_height;
    double move_step;
    double zoom_step;
    int use_rtree_for_drawing;
    int use_double_buffer;
    int use_line_smooth;
    int multisample_level;
    int mouse_btn_zoomin;
    int mouse_btn_zoomout;
    double zoom_low_limit, zoom_high_limit;

    void target(MapData *md);
    void target_gui(MapGUI *mgui);
    void target_shortestpath(MapShortestPath *msp);
    void target_taxiroute(MapTaxiRoute *mtr);
    void target_operation(MapOperation *mo);
    
    void get_display_range(double *minx, double *maxx, double *miny, double *maxy);
    void push_display_range();
    void pop_display_range();
    void move_display_range(int x, int y);
    void zoom_display_range(int f, bool by_mouse = false);
    //void zoom_display_by_size(double sizex, double sizey);
    void move_display_to_point(double x, double y);
    void reset_display_range();
    void center_way(MapWay *way);
    void center_point(MapNode *node);
    void drag_map();
        
    void redraw();
    void reshape(int width, int height);
    void special_keyevent(int key, int x, int y);
    void keyevent(unsigned char key, int x, int y);
    void mouseupdown(int button, int state, int x, int y);
    void mousemotion(int x, int y);
    void show(const char *title, int argc, char *argv[]);
};
#endif
