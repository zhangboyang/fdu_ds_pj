#include <cstdio>
#include "common.h"
#include "MapPerformanceTest.h"
#include "myclock.h"
#include "printf2str.h"

#include <vector>
#include <algorithm>
using namespace std;

void MapPerformanceTest::target(MapData *md) { this->md = md; }
void MapPerformanceTest::target_shortestpath(MapShortestPath *msp) { this->msp = msp; }

void MapPerformanceTest::test_shortestpath()
{
    vector<MapNode *> npool;
    for (vector<MapNode *>::iterator nit = md->nl.begin(); nit != md->nl.end(); nit++) {
        MapNode *node = *nit;
        if (node->on_shortest_path)
            npool.push_back(node);
    }
    random_shuffle(npool.begin(), npool.end());
    
    const int MAX_SHORTESTPATH_TESTCASE = 40;
    printf("%-35s", "");
    for (int algo = 0; algo < MapShortestPath::ALGO_COUNT; algo++) {
        printf("%-20s", msp->get_algo_name(algo));
    }
    printf("\n");
    
    vector<MapLine *> result;
    double time_sum[MapShortestPath::ALGO_COUNT] = {};
    for (int i = 1; i <= MAX_SHORTESTPATH_TESTCASE; i++) {
        printf("%-3d", i);
        printf("%12lld->%-12lld", npool[i * 2]->id, npool[i * 2 + 1]->id);
        double reference_time = 0;
        for (int algo = 0; algo < MapShortestPath::ALGO_COUNT; algo++) {
            result.clear();
            double mindist, time, prepare_time;
            msp->set_start(npool[i * 2]);
            msp->set_end(npool[i * 2 + 1]);
            msp->get_shortest_path(result, mindist, time, prepare_time, algo);
            if (algo == 0) reference_time = time;
            time_sum[algo] += time;
            printf("%-20s", printf2str("%8.1fms/%.4fx", time, reference_time / time).c_str());
            fflush(stdout);
        }
        printf("\n");
    }
    printf("%-29s", "average");
    double reference_time = time_sum[0] / MAX_SHORTESTPATH_TESTCASE;
    for (int algo = 0; algo < MapShortestPath::ALGO_COUNT; algo++) {
        double time = time_sum[algo] / MAX_SHORTESTPATH_TESTCASE;
        printf("%-20s", printf2str("%8.1fms/%.4fx", time, reference_time / time).c_str());
    }
    printf("\n");
}

void MapPerformanceTest::test_rtree()
{
    const int MAX_RTREE_SLICE_UNIT = 20;
    const int DISPLAY_LIMIT = 4;
    const int REPEAT_LIMIT = 1000;
    vector<MapLine *> result;
    for (int s = 1; s <= MAX_RTREE_SLICE_UNIT; s++) {
        printf("%-20s", printf2str("%d*%d=%d", s, s, s * s).c_str());
        vector<pair<int, double> > test_result;
        for (int i = 0; i < s; i++) {
            for (int j = 0; j < s; j++) {
                
                double diffx = (md->maxx - md->minx) / s;
                double diffy = (md->maxy - md->miny) / s;
                double minx = md->minx + diffx * i;
                double maxx = minx + diffx;
                double miny = md->miny + diffy * j;
                double maxy = miny + diffy;
                double st_clock = myclock();
                for (int r = 0; r < REPEAT_LIMIT; r++) {
                    result.clear();
                    md->lrt[0].find(result, MapRect(minx, maxx, miny, maxy));
                }
                double time = (myclock() - st_clock) / REPEAT_LIMIT;
                test_result.push_back(make_pair((int) result.size(), time));
            }
        }
        sort(test_result.begin(), test_result.end(), greater<pair<int, double> >());
        for (int i = 0; i < DISPLAY_LIMIT && i < (int) test_result.size(); i++) {
            printf("%7.2fms/%-10d", test_result[i].second, test_result[i].first);
        }
        printf("\n");
    }
        
        
}

void MapPerformanceTest::run()
{
    double st_clock = myclock();
    
    timing_start("shortest path");
    test_shortestpath();
    timing_end();

    timing_start("r-tree query");
    
    test_rtree();
    timing_end();
    
    double ed_clock = myclock();
    printf("all test finished in %.2f s\n", (ed_clock - st_clock) / 1000);
}
