#ifndef ZBY_MAPGRAPHICS_H
#define ZBY_MAPGRAPHICS_H
#include "common.h"
#include "MapObject.h"
#include "MapData.h"

class MapGraphics {
    private:
    void init_glut();
    
    public:
    static const int WINDOWHEIGHT = 500;
    double gminx, gmaxx, gminy, gmaxy;
    MapData *md;
    
    void target(MapData *md);
    void get_gcoord_by_node(MapNode *node, double *gx, double *gy);
    void set_glimit(double gminx, double gmaxx, double gminy, double gmaxy);
    void redraw();
    void show(const char *title, int argc, char *argv[]);
};
#endif
