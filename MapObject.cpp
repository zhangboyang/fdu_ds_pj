#include <cstdio>
#include "common.h"
#include "MapObject.h"

/* MapObject */
void MapObject::set_id(LL id) { MapObject::id = id; }

/* MapNode */
void MapNode::set_coord(double lat, double lon) { MapNode::lat = lat; MapNode::lon = lon; trans_coord(lat, lon, &x, &y); }
void MapNode::trans_coord(double lat, double lon, double *x, double *y)
{
    assert(x); assert(y);
    *x = lat; // FIXME
    *y = lon;
}

/* MapWay */
void MapWay::add_node(MapNode *node) { nl.push_back(node); }

/* MapRelation */

