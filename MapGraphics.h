#ifndef ZBY_MAPGRAPHICS_H
#define ZBY_MAPGRAPHICS_H
#include "common.h"
#include "MapObject.h"
#include "MapData.h"

class MapGraphics {
    private:
    void init_glut();
    
    public:
    
    enum MapGraphicsOperation {
        UP, DOWN, LEFT, RIGHT,
        ZOOMOUT, ZOOMIN,
    };
    static const int WINDOWHEIGHT = 500;
    static const double MOVESTEP = 0.1;
    static const double ZOOMSTEP = 0.8;
    
    double gminx, gmaxx, gminy, gmaxy;
    MapData *md;
    
    void target(MapData *md);
    void get_gcoord_by_node(MapNode *node, double *gx, double *gy);
    void set_glimit(double gminx, double gmaxx, double gminy, double gmaxy);
    void move_glimit(int x, int y);
    void zoom_glimit(int f);
    void map_operation(MapGraphicsOperation op);
    
    void redraw();
    void special_keyevent(int key, int x, int y);
    void show(const char *title, int argc, char *argv[]);
};
#endif
