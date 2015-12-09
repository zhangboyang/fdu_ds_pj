#ifndef ZBY_MAPGRAPHICS_H
#define ZBY_MAPGRAPHICS_H
#include "common.h"
#include "MapObject.h"
#include "MapData.h"

class MapGraphics {
    private:
    enum MapGraphicsOperation {
        UP, DOWN, LEFT, RIGHT,
        ZOOMOUT, ZOOMIN,
        RESETVIEW,
    };
    
    double dminx, dmaxx, dminy, dmaxy;
    int window_width, window_height;
    int zoom_level;
    MapData *md;
    
    void trans_gcoord(double x, double y, double *gx, double *gy);
    void rtrans_gcoord(double gx, double gy, double *x, double *y);
    
    double get_display_resolution();
    int get_display_level_limit();
    void set_display_range(double dminx, double dmaxx, double dminy, double dmaxy);
    void move_display_range(int x, int y);
    void zoom_display_range(int f);
    void reset_display_range();
    void map_operation(MapGraphicsOperation op);
    
    public:
    static const int INITIAL_WINDOW_HEIGHT = 500;
    static const double MOVESTEP = 0.1;
    static const double ZOOMSTEP = 0.8;

    void target(MapData *md);
    void add_display_level(double res);
    
    void redraw();
    void reshape(int width, int height);
    void special_keyevent(int key, int x, int y);
    void show(const char *title, int argc, char *argv[]);
};
#endif
