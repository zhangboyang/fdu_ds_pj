#include "common.h"
#include "MapShortestPath.h"
#include "myclock.h"

#include <deque>
using namespace std;

// marcos used by algorithms
typedef MapNode *vertex_t;
typedef MapLine *edge_t;
typedef double dist_t;
#define dist_inf F_INF

// iterator between edges which start by 'u', the iterator is 'it'
#define iterate_edges(u, it) for (vector<edge_t>::iterator it = (u)->edges.begin(); it != (u)->edges.end(); it++)
#define cur_edge(it) (*it) // get current edge by iterator 'it'

#define edge_v1(e) ((e)->p1) // get one of vertex of an edge, note: it might NOT be the start of an edge, since it is undirected edge
#define edge_v2(e) ((e)->p2) // get one of vertex of an edge
#define opp_vertex(e, u) ((vertex_t) ((long) edge_v1(e) ^ (long) edge_v2(e) ^ (long) (u))) // get the opposite vertex of an edge

#define edge_len(e) ((e)->len) // get edge length

#define dist(v) ((v)->dist) // distance from source vertex, default is dist_inf
#define from(v) ((v)->from) // save from which edge can goto this vertex, default is NULL
#define flag(v) ((v)->flag) // save algorithm specific flags, default is 0


// the SPFA algorithm
#define inqueue(v) (flag(v))
#define set_inqueue(v, inq) (inqueue(v) = (inq))
void MapShortestPath::run_spfa()
{
    deque<vertex_t> q;
    q.push_back(S);
    set_inqueue(S, 1);
    dist(S) = 0;
    
    while (!q.empty()) {
        vertex_t u = q.front();
        q.pop_front();
        set_inqueue(u, 0);
        if (dist(u) < dist(T)) {
            iterate_edges(u, it) {
                edge_t e = cur_edge(it);
                vertex_t v = opp_vertex(e, u);
                dist_t len = edge_len(e);
                if (dist(u) + len < dist(v)) {
                    dist(v) = dist(u) + len;
                    from(v) = e;
                    if (!inqueue(v)) {
                        q.push_back(v);
                        set_inqueue(u, 1);
                    }
                }
            }
        }
    }
}






// prepare for shortest path algorithm, set default value to every node
void MapShortestPath::prepare()
{
    for (vector<vertex_t>::iterator nit = md->nl.begin(); nit != md->nl.end(); nit++) {
        vertex_t v = *nit;
        dist(v) = dist_inf;
        flag(v) = 0;
        from(v) = NULL;
    }
}

// get shortest path results, if no path, return false
//   save edges to 'result', save minimal distance to 'mindist'
bool MapShortestPath::get_result(std::vector<MapLine *> &result, double &mindist)
{
    assert(S && T && S != T);
    if (!from(T)) return false; // if from(v) is NULL, shortest path algorithm failed
    int offset = result.size();
    for (vertex_t v = T; from(v); v = opp_vertex(from(v), v))
        result.push_back(from(v));
    reverse(result.begin() + offset, result.end());
    mindist = dist(T);
    return true;
}

// run algorithm pointed by function pointer result
bool MapShortestPath::run_algorithm(algo_ptr algo, std::vector<MapLine *> &result, double &mindist, double &time)
{
    assert(S && T && S != T);
    double st = myclock();
    prepare();
    (this->*algo)();
    if (!get_result(result, mindist)) return false;
    time = myclock() - st;
    return true;
}

MapShortestPath::algo_ptr MapShortestPath::get_algo_ptr(int type)
{
    switch (type) {
        case ALGO_SPFA: return &MapShortestPath::run_spfa;
        default: assert(0); return NULL;
    }
}
const char *MapShortestPath::get_algo_name(int type)
{
    switch (type) {
        case ALGO_SPFA: return "SPFA";
        case ALGO_DIJKSTRA: return "Dijkstra";
        case ALGO_ASTAR: return "A*";
        default: assert(0); return NULL;
    }
}

void MapShortestPath::target(MapData *md) { this->md = md; }
void MapShortestPath::set_start(MapNode *S) { this->S = S; }
void MapShortestPath::set_end(MapNode *T) { this->T = T; }

bool MapShortestPath::get_shortest_path(std::vector<MapLine *> &result, double &mindist, double &time, int type)
{
    assert(S && T && S != T);
    assert(md);
    assert(sizeof(long) == sizeof(void *)); // for opp_vertex()
    algo_ptr algo = get_algo_ptr(type);
    assert(algo);
    return run_algorithm(algo, result, mindist, time);
}

