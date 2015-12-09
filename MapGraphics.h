#ifndef ZBY_MAPGRAPHICS_H
#define ZBY_MAPGRAPHICS_H
#include "common.h"
#include "MapObject.h"
#include "MapData.h"

class MapGraphics {
    private:
    
    public:
    
    enum MapGraphicsOperation {
        UP, DOWN, LEFT, RIGHT,
        ZOOMOUT, ZOOMIN,
        RESETVIEW,
    };
    static const int INITIAL_WINDOW_HEIGHT = 500;
    static const double MOVESTEP = 0.1;
    static const double ZOOMSTEP = 0.8;
    
    double dminx, dmaxx, dminy, dmaxy;
    int window_width, window_height;
    MapData *md;
    
    void target(MapData *md);
    void trans_gcoord(double x, double y, double *gx, double *gy);
    void set_display_range(double dminx, double dmaxx, double dminy, double dmaxy);
    void move_display_range(int x, int y);
    void zoom_display_range(int f);
    void reset_display_range();
    void map_operation(MapGraphicsOperation op);
    
    void redraw();
    void reshape(int width, int height);
    void special_keyevent(int key, int x, int y);
    void show(const char *title, int argc, char *argv[]);
};
#endif
