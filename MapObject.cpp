#include <cstdio>
#include "common.h"
#include "MapObject.h"
#include "MapRect.h"

#include <algorithm>
using namespace std;

/* MapObject */
void MapObject::set_id(LL id) { MapObject::id = id; }
//MapObject::ObjectType MapObject::type() { return NONE; }
//MapObject::~MapObject() {}

/* MapNode */
//MapObject::ObjectType MapNode::type() { return NODE; }

/* MapLine */
void MapLine::set_line(MapNode *p1, MapNode *p2)
{
    MapLine::p1 = p1;
    MapLine::p2 = p2;
    //x1 = p1->x; y1 = p1->y;
    //x2 = p2->x; y2 = p2->y;
}
void MapLine::set_way(MapWay *way)
{
    MapLine::way = way;
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
//MapObject::ObjectType MapWay::type() { return WAY; }
void MapWay::add_node(MapNode *node) { nl.push_back(node); }

/* MapRelation */
//MapObject::ObjectType MapRelation::type() { return RELATION; }
// FIXME: 'relation' not implemented
