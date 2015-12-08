#include <cstdio>
#include <cmath>
#include "common.h"
#include "MapData.h"

#include <utility>
#include <algorithm>
using namespace std;

/* MapData */
MapData::~MapData()
{
    printd("destructing mapdata object %p\n", this);
    for (std::vector<MapNode *>::iterator it = nl.begin(); it != nl.end(); it++)
        delete *it;
    for (std::vector<MapWay *>::iterator it = wl.begin(); it != wl.end(); it++)
        delete *it;
    for (std::vector<MapRelation *>::iterator it = rl.begin(); it != rl.end(); it++)
        delete *it;
}

void MapData::insert(MapNode *node) { nl.push_back(node); nm.insert(make_pair(node->id, node)); }
void MapData::insert(MapWay *way) { wl.push_back(way); wm.insert(make_pair(way->id, way)); }
void MapData::insert(MapRelation *rela) { rl.push_back(rela); rm.insert(make_pair(rela->id, rela)); }

void MapData::set_coord_limit(double minlat, double maxlat, double minlon, double maxlon)
{
    MapData::minlat = minlat;
    MapData::maxlat = maxlat;
    MapData::minlon = minlon;
    MapData::maxlon = maxlon;
    geo_ratio = 1 / cos(fabs(maxlat + minlat) / 360 * M_PI);
    assert(geo_ratio >= 1);
    
    /* since we set maxmin, gratio first, we can call trans_coord() now */
    trans_coord(minlat, minlon, &minx, &miny);
    trans_coord(maxlat, maxlon, &maxx, &maxy);
    assert(maxx > minx); //if (minx > maxx) swap(minx, maxx);
    assert(maxy > miny); //if (miny > maxy) swap(miny, maxy);
    map_ratio = (maxy - miny) / (maxx - minx);
}

void MapData::trans_coord(double lat, double lon, double *x, double *y)
{
    /* must call set_coord_limit() first */
    assert(x); assert(y);
    *x = lat;
    *y = lon / geo_ratio;
}

void MapData::set_node_coord_by_geo(MapNode *node, double lat, double lon)
{
    node->lat = lat;
    node->lon = lon;
    trans_coord(lat, lon, &node->x, &node->y);
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
    printf(" map_ratio: %f  geo_ratio: %f\n", map_ratio, geo_ratio);
    printf(" node count: %d\n", (int) nl.size());
    printf(" way count: %d\n", (int) wl.size());
    printf(" relation count: %d\n", (int) rl.size());

#ifdef DEBUG
#define memsz(sz, esz) ((LL) (sz) * esz / 1048576.0)
#define printsz(fmt, sz, esz) printf(" " fmt "%lld(%.2fmb)", (LL) (sz), memsz(sz, esz))

    printf("map memory statistics:\n");
    printf(" mapobject:");
    printsz("mapnode: ", nl.size(), sizeof(MapNode));
    printsz("mapway: ", wl.size(), sizeof(MapWay));
    printf("\n");
    printf(" mapdata vector:");
    printsz("nl: ", nl.capacity(), sizeof(void *));
    printsz("wl: ", wl.capacity(), sizeof(void *));
    printsz("rl: ", rl.capacity(), sizeof(void *));
    printf("\n");
    printf(" mapdata map (approx.) :");
    printsz("nm: ", nm.size(), sizeof(void *));
    printsz("wm: ", wm.size(), sizeof(void *));
    printsz("rm: ", rm.size(), sizeof(void *));
    printf("\n");
    
    LL cnt = 0;
    for (std::vector<MapWay *>::iterator it = wl.begin(); it != wl.end(); it++) {
        cnt += (*it)->nl.capacity();
    }
    
    printf(" mapway vector:"); printsz("sum: ", cnt, sizeof(void *)); printf("\n");
#endif
}
