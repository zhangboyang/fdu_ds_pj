#ifndef ZBY_MAPGRAPHICS_H
#define ZBY_MAPGRAPHICS_H
#include "common.h"
#include "MapObject.h"
#include "MapData.h"

class MapGraphics {
    public:
    static const double MAPWIDTH = 1.0;
    static const double MAPHEIGHT = 1.0;
    static const int WINDOWHEIGHT = 500;
    MapData *md;
    
    void target(MapData *md);
    void redraw();
    void show(const char *title, int argc, char *argv[]);
};
#endif
