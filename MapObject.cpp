#include <cstdio>
#include "common.h"
#include "MapObject.h"

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

/* MapWay */
//MapObject::ObjectType MapWay::type() { return WAY; }
void MapWay::add_node(MapNode *node) { nl.push_back(node); }

/* MapRelation */
//MapObject::ObjectType MapRelation::type() { return RELATION; }
// FIXME: 'relation' not implemented
