#include <cstdio>
#include "common.h"
#include "MapObject.h"
#include "MapRect.h"
#include "MapVector.h"

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
MapNode::MapNode()
{
    on_shortest_path = false;
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
bool MapWay::is_closed() { return !nl[0].empty() && nl[0].front() == nl[0].back(); }
double MapWay::calc_length()
{
    double way_length = 0;
    MapPoint A(nl[0].front()->x, nl[0].front()->y), B;
    for (vector<MapNode *>::iterator it = ++nl[0].begin(); it != nl[0].end(); it++) {
        MapNode *node = *it;
        B = MapPoint(node->x, node->y);
        if (it != nl[0].begin()) {
            way_length += len(B - A);
        }
        A = B;
    }
    return way_length;
}
double MapWay::calc_area()
{
    assert(is_closed());
    double way_area = 0;
    MapPoint P(nl[0].front()->x, nl[0].front()->y);
    MapPoint A(P), B;
    for (vector<MapNode *>::iterator it = ++nl[0].begin(); it != nl[0].end(); it++) {
        MapNode *node = *it;
        B = MapPoint(node->x, node->y);
        way_area += det(B - A, B - P);
        A = B;
    }
    return fabs(way_area) / 2;
}
MapWay::MapWay()
{
    nl.resize(1); // there is at least one map level
    waytype = 0; // set to default waytype
    on_shortest_path = one_way = false;
}

/* MapRelation */
// FIXME: 'relation' not implemented
