#include "common.h"
#include "MapShortestPath.h"
#include "myclock.h"
#include "MapVector.h"

#include <queue>
#include <set>
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
void MapShortestPath::run_spfa()
{
    queue<vertex_t> q;
    q.push(S);
    inqueue(S) = 1;
    dist(S) = 0;
    
    while (!q.empty()) {
        vertex_t u = q.front();
        q.pop();
        inqueue(u) = 0;
        if (dist(u) < dist(T)) {
            iterate_edges(u, it) {
                edge_t e = cur_edge(it);
                vertex_t v = opp_vertex(e, u);
                dist_t len = edge_len(e);
                if (dist(u) + len < dist(v)) {
                    dist(v) = dist(u) + len;
                    from(v) = e;
                    if (!inqueue(v)) {
                        q.push(v);
                        inqueue(v) = 1;
                    }
                }
            }
        }
    }
}
#undef inqueue

// the Dijkstra algorithm, using set as priority queue
class comp_by_dist {
    public:
    bool operator () (vertex_t a, vertex_t b)
    {
        return dist(a) < dist(b);
    }
};
void MapShortestPath::run_dijkstra()
{
    set<vertex_t, comp_by_dist> q;
    dist(S) = 0;
    q.insert(S);
    
    while (!q.empty()) {
        vertex_t u = *q.begin();
        flag(u) = 1;
        if (u == T) break;
        q.erase(q.begin());
        iterate_edges(u, it) {
            edge_t e = cur_edge(it);
            vertex_t v = opp_vertex(e, u);
            if (!flag(v)) {
                dist_t len = edge_len(e);
                if (dist(u) + len < dist(v)) {
                    q.erase(v); // erase first since we want to change dist(v)
                    dist(v) = dist(u) + len;
                    from(v) = e;
                    q.insert(v);
                }
            }
        }
    }
}


// the A-star algorithm
#define f(u, v) (vlen(MapPoint(u) - MapPoint(v)))
//#define f(u, v) (0)
#define estimated_dist(v) ((v)->estimated_dist) // NOT initialized
class comp_by_estimated_dist {
    public:
    bool operator () (vertex_t a, vertex_t b)
    {
        return estimated_dist(a) < estimated_dist(b);
    }
};
void MapShortestPath::run_astar()
{
    set<vertex_t, comp_by_estimated_dist> q;
    dist(S) = 0;
    estimated_dist(S) = 0;
    q.insert(S);
    
    while (!q.empty()) {
        vertex_t u = *q.begin();
        flag(u) = 1;
        if (u == T) break;
        q.erase(q.begin());
        iterate_edges(u, it) {
            edge_t e = cur_edge(it);
            vertex_t v = opp_vertex(e, u);
            if (!flag(v)) {
                dist_t len = edge_len(e);
                if (dist(u) + len < dist(v)) {
                    q.erase(v); // erase first since we want to change estimated_dist(v)
                    dist(v) = dist(u) + len;
                    estimated_dist(v) = dist(v) + f(v, T);
                    from(v) = e;
                    q.insert(v);
                }
            }
        }
    }
}
#undef f
#undef estimated_dist



// the BFS algorithm, note it's will not calculate the shortest path
#define vis(v) (flag(v))
void MapShortestPath::run_bfs()
{
    queue<vertex_t> q;
    dist(S) = 0;
    q.push(S);
    vis(S) = 1;
    
    while (!q.empty()) {
        vertex_t u = q.front();
        if (u == T) break;
        q.pop();
        iterate_edges(u, it) {
            edge_t e = cur_edge(it);
            vertex_t v = opp_vertex(e, u);
            if (!vis(v)) {
                dist(v) = dist(u) + edge_len(e);
                from(v) = e;
                vis(v) = 1;
                q.push(v);
            }
        }
    }
}
#undef vis


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
bool MapShortestPath::run_algorithm(algo_ptr algo, std::vector<MapLine *> &result, double &mindist, double &time, double &prepare_time)
{
    assert(S && T && S != T);
    double t1 = myclock();
    prepare();
    double t2 = myclock();
    prepare_time = t2 - t1;
    (this->*algo)();
    time = myclock() - t2;
    if (!get_result(result, mindist)) return false;
    return true;
}

MapShortestPath::algo_ptr MapShortestPath::get_algo_ptr(int type)
{
    switch (type) {
        case ALGO_SPFA: return &MapShortestPath::run_spfa;
        case ALGO_DIJKSTRA: return &MapShortestPath::run_dijkstra;
        case ALGO_ASTAR: return &MapShortestPath::run_astar;
        case ALGO_BFS: return &MapShortestPath::run_bfs;
        default: assert(0); return NULL;
    }
}
const char *MapShortestPath::get_algo_name(int type)
{
    switch (type) {
        case ALGO_SPFA: return "SPFA";
        case ALGO_DIJKSTRA: return "Dijkstra";
        case ALGO_ASTAR: return "A*";
        case ALGO_BFS: return "BFS";
        default: assert(0); return NULL;
    }
}

void MapShortestPath::target(MapData *md) { this->md = md; }
void MapShortestPath::set_start(MapNode *S) { this->S = S; }
void MapShortestPath::set_end(MapNode *T) { this->T = T; }

bool MapShortestPath::get_shortest_path(std::vector<MapLine *> &result, double &mindist, double &time, double &prepare_time, int type)
{
    assert(S && T && S != T);
    assert(md);
    assert(sizeof(long) == sizeof(void *)); // for opp_vertex()
    algo_ptr algo = get_algo_ptr(type);
    assert(algo);
    return run_algorithm(algo, result, mindist, time, prepare_time);
}

