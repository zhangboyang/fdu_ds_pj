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
    for (std::vector<MapLine *>::iterator it = ll.begin(); it != ll.end(); it++)
        delete *it;
    for (std::vector<MapWay *>::iterator it = wl.begin(); it != wl.end(); it++)
        delete *it;
    for (std::vector<MapRelation *>::iterator it = rl.begin(); it != rl.end(); it++)
        delete *it;
}

void MapData::insert(MapNode *node) { nl.push_back(node); nm.insert(make_pair(node->id, node)); }
void MapData::insert(MapLine *line) { ll.push_back(line); }
void MapData::insert(MapWay *way) { wl.push_back(way); wm.insert(make_pair(way->id, way)); }
void MapData::insert(MapRelation *rela) { rl.push_back(rela); rm.insert(make_pair(rela->id, rela)); }

void MapData::set_coord_limit(double minlat, double maxlat, double minlon, double maxlon)
{
    MapData::minlat = minlat;
    MapData::maxlat = maxlat;
    MapData::minlon = minlon;
    MapData::maxlon = maxlon;
    geo_factor = cos(fabs(maxlat + minlat) * (M_PI / 360));
    
    double maxlat_factor = cos(fabs(maxlat) * (2 * M_PI / 360));
    double minlat_factor = cos(fabs(minlat) * (2 * M_PI / 360));
    double maxlat_error = maxlat_factor / geo_factor - 1;
    double minlat_error = minlat_factor / geo_factor - 1;
    //printf("%f %f %f %f %f\n", geo_factor, maxlat_factor, minlat_factor, maxlat_error, minlat_error);
    max_geo_error = fabs(maxlat_error) > fabs(minlat_error) ? maxlat_error : minlat_error;
    
    assert(0 < geo_factor && geo_factor <= 1);
    
    /* since we set maxmin, gratio first, we can call trans_coord() now */
    trans_coord(minlat, minlon, &minx, &miny);
    trans_coord(maxlat, maxlon, &maxx, &maxy);
    assert(maxx > minx); //if (minx > maxx) swap(minx, maxx);
    assert(maxy > miny); //if (miny > maxy) swap(miny, maxy);
    map_ratio = (maxx - minx) / (maxy - miny);
}

void MapData::trans_coord(double lat, double lon, double *x, double *y)
{
    /* must call set_coord_limit() first */
    assert(x); assert(y);
    *x = lon * geo_factor;
    *y = lat;
}

void MapData::set_node_coord_by_geo(MapNode *node, double lat, double lon)
{
    //node->lat = lat;
    //node->lon = lon;
    trans_coord(lat, lon, &node->x, &node->y);
}

MapNode *MapData::get_node_by_id(LL id)
{
    map<LL, MapNode *>::iterator it = nm.find(id);
    if (it == nm.end()) fail("node object %lld not found", id);
    return it->second;
}

void MapData::construct_line_by_signal_way(MapWay *way)
{
    MapNode *last_node = NULL;
    for (vector<MapNode *>::iterator nit = way->nl.begin(); nit != way->nl.end(); nit++) {
        MapNode *node = *nit;
        if (last_node) {
            MapLine *line = new MapLine;
            line->set_line(last_node, node);
            line->set_way(way);
            insert(line);
        }
        last_node = node;
    }
}

void MapData::construct()
{
    assert(nl.size() > 0 && nl.size() == nm.size());
    assert(wl.size() > 0 && wl.size() == wm.size());
    //assert(rl.size() > 0 && rl.size() == rm.size());
    assert(ll.size() == 0);
    
    TIMING ("mapdata construct", {
    
        // construct lines
        for (vector<MapWay *>::iterator wit = wl.begin(); wit != wl.end(); wit++)
            construct_line_by_signal_way(*wit);
        
        // put lines to r-tree
        assert(ml.get_level_count() > 0);
        lrt.resize(ml.get_level_count());
        for (vector<MapLine *>::iterator lit = ll.begin(); lit != ll.end(); lit++) {
            MapLine *line = *lit;
            int slvl = wt.query_level(line->way->waytype); // suggested level
            if (slvl == -1) {
                double res = line->get_rect().max_distance() * dfactor;
                slvl = ml.select_level(res);
            } else if (slvl == -2) {
                slvl = ml.get_level_count() - 1;
            }
            assert(slvl >= 0);
            for (int lvl = 0; lvl <= slvl; lvl++)
                lrt[lvl].insert(line);
        }
    })
}

void MapData::print_stat()
{
    printf("map data statistics:\n");
    printf(" minlat: %f(%f)  maxlat: %f(%f)\n", minlat, minx, maxlat, maxx);
    printf(" minlon: %f(%f)  maxlon: %f(%f)\n", minlon, miny, maxlon, maxy);
    printf(" map_ratio: %f  geo_factor: %f\n", map_ratio, geo_factor);
    printf(" max_geo_error: %.2f%%\n", max_geo_error * 100);
    printf(" node count: %d\n", (int) nl.size());
    printf(" way count: %d\n", (int) wl.size());
    printf(" relation count: %d\n", (int) rl.size());

#ifdef DEBUG
#define memsz(sz, esz) ((LL) (sz) * (esz) / 1048576.0)
#define printsz(fmt, sz, esz) printf(" " fmt "%lld(%.2fmb)", (LL) (sz), memsz((sz), (esz)))

    printf("object size statistics:\n");
    printf(" mapobject: %lld\n", (LL) sizeof(MapObject));
    printf(" mapnode: %lld\n", (LL) sizeof(MapNode));
    printf(" mapline: %lld\n", (LL) sizeof(MapLine));
    printf(" mapway: %lld\n", (LL) sizeof(MapWay));
    printf(" mapdata: %lld\n", (LL) sizeof(MapData));
    printf(" std::vector<void *>: %lld\n", (LL) sizeof(vector<void *>));
    printf(" std::map<LL, void *>: %lld\n", (LL) sizeof(map<LL, void *>));
    printf("map memory statistics:\n");
    printf(" mapobject:");
    printsz("node: ", nl.size(), sizeof(MapNode));
    printsz("way: ", wl.size(), sizeof(MapWay));
    printsz("line: ", ll.size(), sizeof(MapLine));
    printf("\n");
    printf(" mapdata vector:");
    printsz("nl: ", nl.size(), sizeof(void *));
    printsz("wl: ", wl.size(), sizeof(void *));
    //printsz("rl: ", rl.size(), sizeof(void *));
    printsz("ll: ", ll.size(), sizeof(void *));
    printf("\n");
    printf(" mapdata map (approx.) :");
    printsz("nm: ", nm.size(), sizeof(void *));
    printsz("wm: ", wm.size(), sizeof(void *));
    //printsz("rm: ", rm.size(), sizeof(void *));
    printf("\n");
    
    LL cnt = 0;
    for (std::vector<MapWay *>::iterator it = wl.begin(); it != wl.end(); it++) {
        cnt += (*it)->nl.capacity();
    }
    
    printf(" mapway vector:"); printsz("sum: ", cnt, sizeof(void *)); printf("\n");
#endif
}
