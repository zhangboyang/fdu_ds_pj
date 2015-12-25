#ifndef ZBY_SHORTESTPATH_H
#define ZBY_SHORTESTPATH_H
#include "common.h"
#include "MapData.h"
#include <vector>
#include <string>

class MapShortestPath {
    public:
    enum algo_type {
        ALGO_ASTAR,
        ALGO_DIJKSTRA,
        ALGO_SPFA,
        ALGO_BFS,
        ALGO_COUNT, // must be last one
    };
    
    private:
    MapNode *S, *T; // start node, end node
    
    typedef void (MapShortestPath::*algo_ptr)();
    void run_dijkstra();
    void run_spfa();
    void run_astar();
    void run_bfs();
    
    bool get_result(std::vector<MapLine *> &result, double &mindist); // return false is there is no path, append edges to result
    bool run_algorithm(algo_ptr algo, std::vector<MapLine *> &result, double &mindist, double &time, double &prepare_time);
    
    algo_ptr get_algo_ptr(int type);
    
    public:
    MapData *md;
    
    void target(MapData *md);
    void prepare(); // init
    
    void set_start(MapNode *S);
    void set_end(MapNode *T);
    const char *get_algo_name(int type);
    
    // shortest-path, append edges to 'result', save distance to 'mindist', save algo time to 'time', using algorithm 'algo'
    //   return false if no route from start to end
    bool get_shortest_path(std::vector<MapLine *> &result, double &mindist, double &time, double &prepare_time, int type);
};

#endif
