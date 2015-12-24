#include <cstdio>
#include <cmath>
#include "common.h"
#include "MapData.h"
#include "MapVector.h"
#include "printf2str.h"

#include <utility>
#include <algorithm>
using namespace std;

/* MapData */
MapData::~MapData()
{
    printd("destructing mapdata object %p\n", this);
    for (std::vector<MapNode *>::iterator it = nl.begin(); it != nl.end(); it++)
        delete *it;
    for (int lvl = 0; lvl < tot_lvl; lvl++)
        for (std::vector<MapLine *>::iterator it = ll[lvl].begin(); it != ll[lvl].end(); it++)
            delete *it;
    for (std::vector<MapWay *>::iterator it = wl.begin(); it != wl.end(); it++)
        delete *it;
    for (std::vector<MapRelation *>::iterator it = rl.begin(); it != rl.end(); it++)
        delete *it;
    delete[] lrt;
    delete[] ntrt;
    delete[] wtrt;
}

void MapData::insert(MapNode *node) { nl.push_back(node); nm.insert(make_pair(node->id, node)); }
void MapData::insert(MapWay *way) { wl.push_back(way); wm.insert(make_pair(way->id, way)); }
void MapData::insert(MapRelation *rela) { rl.push_back(rela); rm.insert(make_pair(rela->id, rela)); }
void MapData::insert_with_tag(MapNode *node, int tagid) { ntl[tagid].push_back(node); }
void MapData::insert_with_tag(MapWay *way, int tagid) { wtl[tagid].push_back(way); }
bool MapData::tag_key_is_name(const std::string &key)
{
    return key.compare(0, 4, "name") == 0 || key.compare(0, 8, "alt_name") == 0 || key.compare(0, 8, "old_name") == 0;
}

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

std::string MapData::get_area_string(double area)
{
    if (area >= 1e3)
        return printf2str("%.3f km2", area * 1e-6);
    else
        return printf2str("%.2f m2", area);
}

std::string MapData::get_length_string(double length)
{
    if (length >= 1e3)
        return printf2str("%.2f km", length * 1e-3);
    else
        return printf2str("%.2f m", length);
}

void MapData::prepare()
{
    ntl.resize(mt.get_count());
    wtl.resize(mt.get_count());
}

void MapData::construct()
{
    assert(nl.size() > 0 && nl.size() == nm.size());
    assert(wl.size() > 0 && wl.size() == wm.size());
    //assert(rl.size() > 0 && rl.size() == rm.size());
    assert(ll.size() == 0);
    
    timing_start("mapdata construct");
        
        // fetch level count
        tot_lvl = ml.get_level_count();
        assert(tot_lvl > 0);
        ll.resize(tot_lvl);
        
        // calc way's rectangles
        for (vector<MapWay *>::iterator wit = wl.begin(); wit != wl.end(); wit++) {
            MapWay *way = *wit;
            MapRect &rect = way->rect;
            assert(way->nl[0].size() > 0);
            rect.left = rect.right = way->nl[0].front()->x;
            rect.bottom = rect.top = way->nl[0].front()->y;
            for (vector<MapNode *>::iterator nit = ++way->nl[0].begin(); nit != way->nl[0].end(); nit++) {
                MapNode *node = *nit;
                rect.left = min(rect.left, node->x);
                rect.right = max(rect.right, node->x);
                rect.bottom = min(rect.bottom, node->y);
                rect.top = max(rect.top, node->y);
            }
        }
        
        // alloc memory for node list of each level in ways
        for (vector<MapWay *>::iterator wit = wl.begin(); wit != wl.end(); wit++) {
            MapWay *way = *wit;
            way->nl.resize(tot_lvl);
        }
        
        // calc way's level
        for (vector<MapWay *>::iterator wit = wl.begin(); wit != wl.end(); wit++) {
            MapWay *way = *wit;
            int slvl = wt.query_level(way->waytype); // suggested level
            if (slvl == -1) {
                double res = way->get_rect().max_distance() / dfactor;
                slvl = ml.select_level(res);
            } else if (slvl == -2) {
                slvl = tot_lvl - 1;
            }
            assert(slvl >= 0);
            way->level = slvl;
        }
        
        // calc way's node list by level
        for (vector<MapWay *>::iterator wit = wl.begin(); wit != wl.end(); wit++) {
            MapWay *way = *wit;
            vector<MapNode *> &wnl = way->nl[0];
            assert(wnl.size() > 0);
            for (int lvl = 1; lvl <= way->level; lvl++) {
                double res = ml.get_level(lvl);
                double low_res = res * line_detail_dist_low_limit_factor;
                double high_res = res * line_detail_dist_high_limit_factor;
                MapPoint A, B, L; // L->A->B
                bool Lflag = false;
                A = MapPoint(wnl.front()->x, wnl.front()->y);
                way->nl[lvl].push_back(wnl.front());
                double ang = 0, dist = 0;
                for (vector<MapNode *>::iterator nit = ++wnl.begin(); nit != wnl.end(); nit++) {
                    MapNode *node = *nit;
                    B = MapPoint(node->x, node->y);
                    dist += vlen(B - A);
                    if (Lflag) 
                        ang = fabs(angle(B - A, A - L));
                    else
                        Lflag = true;
                    if (*nit == wnl.back() || // last point must included
                            dist >= high_res ||
                            (dist >= low_res && // distance limit
                             ang >= line_detail_angle_limit / 180.0 * M_PI) // angle limit
                        ) {
                        dist = 0;
                        way->nl[lvl].push_back(node);
                    }
                    L = A;
                    A = B;
                }
            }
        }
        
        // construct lines
        for (int lvl = 0; lvl < tot_lvl; lvl++)
            for (vector<MapWay *>::iterator wit = wl.begin(); wit != wl.end(); wit++) {
                MapWay *way = *wit;
                MapNode *last_node = NULL;
                //MapLine *last_line = NULL;
                for (vector<MapNode *>::iterator nit = way->nl[lvl].begin(); nit != way->nl[lvl].end(); nit++) {
                    MapNode *node = *nit;
                    if (last_node) {
                        MapLine *line = new MapLine;
                        line->set_line(last_node, node);
                        line->set_way(way);
                        //line->prev = last_line;
                        //if (last_line) last_line->next = line;
                        ll[lvl].push_back(line);
                        //last_line = line;
                    }
                    last_node = node;
                }
                //if (last_line) last_line->next = NULL;
            }
        
        // construct edges
        for (vector<MapLine *>::iterator lit = ll[0].begin(); lit != ll[0].end(); lit++) {
            MapLine *line = *lit;
            MapNode *from = line->p1, *to = line->p2;
            if (line->way->on_shortest_path) {
                if (line->way->one_way) {
                    from->edges.push_back(line);
                } else {
                    from->edges.push_back(line);
                    to->edges.push_back(line);
                }
            }
        }
        
        // mark node's on_shortest_path
        for (vector<MapLine *>::iterator lit = ll[0].begin(); lit != ll[0].end(); lit++) {
            MapLine *line = *lit;
            MapNode *from = line->p1, *to = line->p2;
            from->on_shortest_path = to->on_shortest_path = line->way->on_shortest_path;
        }
        
        // calc line(edge) length
        for (vector<MapLine *>::iterator lit = ll[0].begin(); lit != ll[0].end(); lit++) {
            MapLine *line = *lit;
            MapNode *from = line->p1, *to = line->p2;
            MapPoint A(from->x, from->y), B(to->x, to->y);
            line->len = vlen(B - A);
        }
        
        // put lines to r-tree
        lrt = new MapRTree<MapLine *> [tot_lvl];
        for (int lvl = 0; lvl < tot_lvl; lvl++)
            for (vector<MapLine *>::iterator lit = ll[lvl].begin(); lit != ll[lvl].end(); lit++) {
                MapLine *line = *lit;
                lrt[lvl].insert(line);
            }
            
        // unique ntl and wtl
        int tot_tag = ntl.size();
        for (int tagid = 0; tagid < tot_tag; tagid++) {
            sort(ntl[tagid].begin(), ntl[tagid].end());
            ntl[tagid].resize(unique(ntl[tagid].begin(), ntl[tagid].end()) - ntl[tagid].begin());
            sort(wtl[tagid].begin(), wtl[tagid].end());
            wtl[tagid].resize(unique(wtl[tagid].begin(), wtl[tagid].end()) - wtl[tagid].begin());
        }
        
        // put ntl and wtl to r-tree
        assert((int) ntl.size() == tot_tag);
        assert((int) wtl.size() == tot_tag);
        ntrt = new MapRTree<MapNode *> [tot_tag];
        for (int tagid = 0; tagid < tot_tag; tagid++)
            for (vector<MapNode *>::iterator nit = ntl[tagid].begin(); nit != ntl[tagid].end(); nit++) {
                MapNode *node = *nit;
                ntrt[tagid].insert(node);
            }
        wtrt = new MapRTree<MapWay *> [tot_tag];
        for (int tagid = 0; tagid < tot_tag; tagid++)
            for (vector<MapWay *>::iterator wit = wtl[tagid].begin(); wit != wtl[tagid].end(); wit++) {
                MapWay *way = *wit;
                wtrt[tagid].insert(way);
            }
        
        // construct dict
        wd.construct();
        nd.construct();
    
    timing_end();
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
