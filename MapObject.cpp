#include <cstdio>
#include "common.h"
#include "MapObject.h"
#include "MapGraphics.h"

/* MapObject */
void MapObject::set_id(LL id) { MapObject::id = id; }

/* MapNode */
void MapNode::set_coord(double lat, double lon) { MapNode::lat = lat; MapNode::lon = lon; trans_coord(lat, lon, &x, &y); }
void MapNode::trans_coord(double lat, double lon, double *x, double *y)
{
    assert(x); assert(y);
    *x = lat;
    *y = lon;
}

double MapNode::gminx = 0, MapNode::gmaxx = 0;
double MapNode::gminy = 0, MapNode::gmaxy = 0;

void MapNode::set_glimit(double gminx, double gmaxx, double gminy, double gmaxy)
{
    MapNode::gminx = gminx; MapNode::gmaxx = gmaxx; // set limit for MapGraphics
    MapNode::gminy = gminy; MapNode::gmaxy = gmaxy;
}

void MapNode::get_gcoord(double *gx, double *gy) // transform for MapGraphics
{
    assert(gmaxx - gminx > 0); assert(gmaxy - gminy > 0);
    *gx = (y - gminy) * MapGraphics::MAPWIDTH / (gmaxy - gminy);
    *gy = (x - gminx) * MapGraphics::MAPHEIGHT / (gmaxx - gminx);
}

/* MapWay */
void MapWay::add_node(MapNode *node) { nl.push_back(node); }

/* MapRelation */

