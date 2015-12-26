#ifndef ZBY_MAPPERFORMANCETEST_H
#define ZBY_MAPPERFORMANCETEST_H
#include "common.h"
#include "MapData.h"
#include "MapShortestPath.h"

class MapPerformanceTest {
    MapData *md;
    MapShortestPath *msp;
    void test_shortestpath();
    void test_rtree();
    
    public:
    void target(MapData *md);
    void target_shortestpath(MapShortestPath *msp);
    void run();
};

#endif
