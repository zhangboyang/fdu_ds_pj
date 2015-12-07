#include <cstdio>
#include "common.h"
#include "MapData.h"

#include <utility>
using namespace std;

/* MapData */
MapData::~MapData()
{
    for (std::vector<MapNode *>::iterator it = nl.begin(); it != nl.end(); it++)
        delete *it;
    for (std::vector<MapWay *>::iterator it = wl.begin(); it != wl.end(); it++)
        delete *it;
    for (std::vector<MapRelation *>::iterator it = rl.begin(); it != rl.end(); it++)
        delete *it;
}

void MapData::insert(MapNode *node) { nl.push_back(node); nm.insert(make_pair(node->id, node)); }
void MapData::insert(MapWay *way) { wl.push_back(way); wm.insert(make_pair(way->id, way)); }
void MapData::insert(MapRelation *rela) { rl.push_back(rela); rm.insert(make_pair(rela->id, rela));}

void MapData::set_coord_limit(double minlat, double maxlat, double minlon, double maxlon)
{
    MapData::minlat = minlat;
    MapData::maxlat = maxlat;
    MapData::minlon = minlon;
    MapData::maxlon = maxlon;
    MapNode::trans_coord(minlat, minlon, &minx, &miny);
    MapNode::trans_coord(maxlat, maxlon, &maxx, &maxy);
}

MapNode *MapData::get_node_by_id(LL id)
{
    map<LL, MapNode *>::iterator it = nm.find(id);
    if (it == nm.end()) fail("node object %lld not found", id);
    return it->second;
}

void MapData::print_stat()
{
    printf("map data statistics:\n");
    printf(" minlat: %f(%f)  maxlat: %f(%f)\n", minlat, minx, maxlat, maxx);
    printf(" minlon: %f(%f)  maxlon: %f(%f)\n", minlon, miny, maxlon, maxy);
    printf(" node count: %d\n", (int) nl.size());
    printf(" way count: %d\n", (int) wl.size());
    printf(" relation count: %d\n", (int) rl.size());
}
