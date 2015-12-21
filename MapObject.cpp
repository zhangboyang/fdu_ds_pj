#include <cstdio>
#include "common.h"
#include "MapObject.h"
#include "MapRect.h"
//#include "MapVector.h"

#include <algorithm>
using namespace std;

/* MapObject */
void MapObject::set_id(LL id) { MapObject::id = id; }

/* MapNode */
MapRect MapNode::get_rect()
{
    MapRect ret;
    ret.left = ret.right = x;
    ret.bottom = ret.top = y;
    return ret;
}

/* MapLine */
void MapLine::set_way(MapWay *way) { MapLine::way = way; }
void MapLine::set_line(MapNode *p1, MapNode *p2)
{
    MapLine::p1 = p1;
    MapLine::p2 = p2;
}
MapRect MapLine::get_rect()
{
    MapRect ret;
    ret.left = min(p1->x, p2->x);
    ret.right = max(p1->x, p2->x);
    ret.bottom = min(p1->y, p2->y);
    ret.top = max(p1->y, p2->y);
    return ret;
}

/* MapWay */
void MapWay::add_node(MapNode *node) { nl[0].push_back(node); }
MapRect MapWay::get_rect() { return rect; }

MapWay::MapWay() { nl.resize(1); } // there is at least one map level

/* MapRelation */
// FIXME: 'relation' not implemented
