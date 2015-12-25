#include <cstdio>
#include "common.h"
#include "MapOperation.h"
#include "MapGraphics.h"
#include "MapVector.h"
#include "wstr.h"
#include "printf2str.h"
#include "myclock.h"

using namespace std;
void MapOperation::query_timer_start() { qclock = myclock(); }
void MapOperation::query_timer_stop()
{
    qtime = myclock() - qclock;
}

void MapOperation::query_tag_with_filter(const string &tag, const MapRect &baserect,
          bool (MapOperation::*node_filter)(MapNode *),
          bool (MapOperation::*way_filter)(MapWay *))
{
    int tagid = md->mt.query(tag);
    if (tagid >= 0) {
        vector<MapNode *> nr;
        vector<MapWay *> wr;
        query_timer_start();
        md->ntrt[tagid].find(nr, baserect);
        md->wtrt[tagid].find(wr, baserect);
        for (vector<MapNode *>::iterator nit = nr.begin(); nit != nr.end(); nit++)
            if ((this->*node_filter)(*nit)) nresult.push_back(*nit);
        for (vector<MapWay *>::iterator wit = wr.begin(); wit != wr.end(); wit++)
            if ((this->*way_filter)(*wit)) wresult.push_back(*wit);
        query_timer_stop();
        query_report.append(printf2str("Tag: %s\nNode: %d\nWay: %d\nTime: %.2f ms\n",
            tag.c_str(), (int) nresult.size(), (int) wresult.size(), qtime));
    } else {
        mg->msg.append(printf2str("ERROR:\n  Invalid tag \"%s\".\n", tag.c_str()));
        return;
    }
}

bool MapOperation::poly_node_filter(MapNode *node) { return point_in_poly(MapPoint(node->x, node->y), pvl) != 0; }
bool MapOperation::poly_way_filter(MapWay *way)
{
    for (vector<MapNode *>::iterator nit = way->nl[0].begin(); nit != way->nl[0].end(); nit++) {
        MapNode *node = *nit;
        if (point_in_poly(MapPoint(node->x, node->y), pvl))
            return true;
    }
    return false;
}

void MapOperation::query_tag_with_poly()
{
    assert(pvl.size() >= 3);
    mgui->prepare_inputbox();
    mgui->set_inputbox_title(L"Query objects in polygon by tag");
    mgui->set_inputbox_description(L"Tag Name");
    wstring uinput = mgui->show_inputbox();
    if (uinput.length() == 0) return;
    clear_results();
    const wchar_t *wstr = uinput.c_str();
    query_description = L"  Query in polygon by tag: " + uinput;
    query_report = "Type: Polygon\n";
    string tag = ws2s(wstr);
    
    assert(pvl.size() >= 3);
    MapRect base = pvl.front().get_rect();
    for (vector<MapPoint>::iterator it = ++pvl.begin(); it != pvl.end(); it++)
        base.merge(it->get_rect());
    
    query_tag_with_filter(tag, base, &MapOperation::poly_node_filter, &MapOperation::poly_way_filter);
    
    select_results();
    //show_results();
}

MapPoint MapOperation::nearby_center;

bool MapOperation::dist_node_filter(MapNode *node) { return vlensq(MapPoint(node->x, node->y) - nearby_center) <= nearby_distsq; }
bool MapOperation::dist_way_filter(MapWay *way)
{
    for (vector<MapNode *>::iterator nit = way->nl[0].begin(); nit != way->nl[0].end(); nit++) {
        MapNode *node = *nit;
        if (vlensq(MapPoint(node->x, node->y) - nearby_center) <= nearby_distsq)
            return true;
    }
    return false;
}

bool MapOperation::sort_node_by_dist(MapNode *a, MapNode *b)
{
    return vlensq(MapPoint(a->x, a->y) - nearby_center) < vlensq(MapPoint(b->x, b->y) - nearby_center);
}

bool MapOperation::sort_way_by_dist(MapWay *a, MapWay *b)
{
    double alensq = F_INF, blensq = F_INF;
    for (vector<MapNode *>::iterator nit = a->nl[0].begin(); nit != a->nl[0].end(); nit++) {
        MapNode *node = *nit;
        alensq = min(alensq, vlensq(MapPoint(node->x, node->y) - nearby_center));
    }
    for (vector<MapNode *>::iterator nit = b->nl[0].begin(); nit != b->nl[0].end(); nit++) {
        MapNode *node = *nit;
        blensq = min(blensq, vlensq(MapPoint(node->x, node->y) - nearby_center));
    }
    return alensq < blensq;
}

void MapOperation::query_tag_with_dist()
{
    assert(pvl.size() == 2);
    mgui->prepare_inputbox();
    mgui->set_inputbox_title(L"Query nearby objects by tag");
    mgui->set_inputbox_description(L"Tag Name");
    wstring uinput = mgui->show_inputbox();
    if (uinput.length() == 0) return;
    clear_results();
    const wchar_t *wstr = uinput.c_str();
    query_description = L"  Query nearby objects by tag: " + uinput;
    query_report = "Type: Nearby\n";
    string tag = ws2s(wstr);
    
    assert(pvl.size() == 2);
    MapRect base = pvl.front().get_rect();
    MapPoint A(pvl.front().x, pvl.front().y);
    MapPoint B(pvl.back().x, pvl.back().y);
    nearby_center = A;
    nearby_distsq = vlensq(B - A);
    double nearby_dist = sqrt(nearby_distsq);
    base.left -= nearby_dist;
    base.right += nearby_dist;
    base.top += nearby_dist;
    base.bottom -= nearby_dist;
    
    query_tag_with_filter(tag, base, &MapOperation::dist_node_filter, &MapOperation::dist_way_filter);
    
    sort(nresult.begin(), nresult.end(), sort_node_by_dist);
    sort(wresult.begin(), wresult.end(), sort_way_by_dist);
    
    select_results();
    //show_results();
}


bool MapOperation::true_node_filter(MapNode *node) { return true; }
bool MapOperation::true_way_filter(MapWay *way) { return true; }

void MapOperation::query_tag_in_display()
{
    mgui->prepare_inputbox();
    mgui->set_inputbox_title(L"Query objects in sight by tag");
    mgui->set_inputbox_description(L"Tag Name");
    wstring uinput = mgui->show_inputbox();
    if (uinput.length() == 0) return;
    clear_results();
    const wchar_t *wstr = uinput.c_str();
    query_description = L"  Query objects in sight by tag: " + uinput;
    query_report = "Type: Insight\n";
    string tag = ws2s(wstr);
    
    double dminx, dminy, dmaxx, dmaxy;
    mg->get_display_range(&dminx, &dmaxx, &dminy, &dmaxy);
    MapRect base = MapRect(dminx, dmaxx, dminy, dmaxy);
    
    query_tag_with_filter(tag, base, &MapOperation::true_node_filter, &MapOperation::true_way_filter);
    
    select_results();
    //show_results();
}

void MapOperation::query_tag()
{
    if (pvl.size() == 2) query_tag_with_dist();
    else if (pvl.size() >= 3) query_tag_with_poly();
    else query_tag_in_display();
}

void MapOperation::query_name()
{
    mgui->prepare_inputbox();
    mgui->set_inputbox_title(L"Query objects by name");
    mgui->set_inputbox_description(L"Query Name");
    wstring uinput = mgui->show_inputbox();
    if (uinput.length() == 0) return;
    clear_results();
    const wchar_t *wstr = uinput.c_str();
    
    query_description = L"  Query by name: " + uinput;
    query_report = "Type: Name\n";
    
    string str = ws2s(uinput);
    const char *ptr = str.c_str();
    int num_flag = 0;
    LL num = 0;
    while (*ptr && isdigit(*ptr)) ptr++;
    if (!*ptr && ptr != str.c_str()) {
        num_flag = 1;
        sscanf(str.c_str(), "%lld", &num);
        printd("name is number %lld\n", num);
    }
    
    query_timer_start();
    md->nd.find(nresult, wstr);
    if (num_flag && md->nm.find(num) != md->nm.end()) nresult.push_back(md->nm.find(num)->second);
    md->wd.find(wresult, wstr);
    if (num_flag && md->wm.find(num) != md->wm.end()) wresult.push_back(md->wm.find(num)->second);
    query_timer_stop();
    query_report.append(printf2str("Node: %d\nWay: %d\nTime: %.2f ms\n",
            (int) nresult.size(), (int) wresult.size(), qtime));
    
    select_results();
    //show_results();
}

void MapOperation::select_results()
{
    clear_select();
    for (int i = 0; i < MAX_KBDNUM; i++) {
        if (i < (LL) nresult.size()) nnode[i] = nresult[i];
        if (i < (LL) wresult.size()) nway[i] = wresult[i];
    }
}

void MapOperation::msgbox_append_tags(MapObject *ptr)
{
    for (vector<pair<string, wstring> >::iterator it = ptr->tl.begin(); it != ptr->tl.end(); it++)
        if (!md->tag_key_is_name(it->first))
            mgui->set_msgbox_append(L"[tag/" + s2ws(it->first) + L"] " + it->second);
}

void MapOperation::msgbox_append_names(MapObject *ptr)
{
    for (vector<pair<string, wstring> >::iterator it = ptr->tl.begin(); it != ptr->tl.end(); it++)
        if (md->tag_key_is_name(it->first))
            mgui->set_msgbox_append(L"[" + s2ws(it->first) + L"] " + it->second);
}

void MapOperation::show_results()
{
    mgui->prepare_msgbox();
    mgui->set_msgbox_title(L"Query result");
    mgui->set_msgbox_description(L"Here are query results:");
    mgui->set_msgbox_append(L"== Query ==");
    mgui->set_msgbox_append(query_description);
    mgui->set_msgbox_append(s2ws(printf2str("  Query completed in %.2f ms", qtime)));
    mgui->set_msgbox_append(L"");
    
    for (vector<MapNode *>::iterator it = nresult.begin(); it != nresult.end(); it++) {
        MapNode *node = *it;
        mgui->set_msgbox_append(s2ws(printf2str("== Node Result %d : #%lld ==", (int)(it - nresult.begin()), node->id)));
        msgbox_append_names((MapObject *) node);
        msgbox_append_tags((MapObject *) node);
    }
    mgui->set_msgbox_append(L"");
    mgui->set_msgbox_append(L"");
    
    for (vector<MapWay *>::iterator it = wresult.begin(); it != wresult.end(); it++) {
        MapWay *way = *it;
        mgui->set_msgbox_append(s2ws(printf2str("== Way Result %d : #%lld ==", (int)(it - wresult.begin()), way->id)));
        msgbox_append_names((MapObject *) way);
        msgbox_append_tags((MapObject *) way);
    }
    
    mgui->show_msgbox();
}

void MapOperation::clear_results()
{
    nresult.clear();
    wresult.clear();
    query_description = L"  No Query";
    query_report.clear();
    qtime = 0;
}

void MapOperation::show_wayinfo()
{
    if (sway) {
        mgui->prepare_msgbox();
        mgui->set_msgbox_title(L"Way Information");
        mgui->set_msgbox_description(s2ws(printf2str("Here is information about way #%lld", sway->id)));
        mgui->set_msgbox_append(s2ws(printf2str("[id] #%lld", sway->id)));
        
        // way names
        msgbox_append_names((MapObject *) sway);
        
        // way length
        double way_length = sway->calc_length() * MapData::dist_factor;
        mgui->set_msgbox_append(s2ws("[length] " + md->get_length_string(way_length)));
        
        // way area
        if (sway->is_closed()) {
            double way_area = sway->calc_area() * sq(MapData::dist_factor);
            mgui->set_msgbox_append(s2ws("[area] " + md->get_area_string(way_area)));
        }
        
        // way tags
        msgbox_append_tags((MapObject *) sway);
        
        mgui->show_msgbox();
    }
}

void MapOperation::show_nodeinfo()
{
    if (snode) {
        mgui->prepare_msgbox();
        mgui->set_msgbox_title(s2ws("Node Information"));
        mgui->set_msgbox_description(s2ws(printf2str("Here is information about node #%lld", snode->id)));
        mgui->set_msgbox_append(s2ws(printf2str("[id] #%lld", snode->id))); // id
        msgbox_append_names((MapObject *) snode);// node names
        mgui->set_msgbox_append(s2ws(printf2str("[coord] lat=%f lon=%f", snode->lat, snode->lon))); // node coord
        msgbox_append_tags((MapObject *) snode); // node tags
        mgui->show_msgbox();
    }
}

void MapOperation::clear_select()
{
    snode = NULL;
    sway = NULL;
    memset(nnode, 0, sizeof(nnode));
    memset(nway, 0, sizeof(nway));
}

void MapOperation::select_way()
{
    double cx = mg->mx, cy = mg->my;
    double mdistsq = F_INF;
    MapPoint P(cx, cy);
    for (vector<MapLine *>::iterator lit = mg->dll.begin(); lit != mg->dll.end(); lit++) {
        MapLine *line = *lit;
        MapPoint A(line->p1), B(line->p2);
        double distsq = distsq_p2s(P, A, B);
        if (distsq < mdistsq) { mdistsq = distsq; sway = line->way; }
    }
}


void MapOperation::select_point()
{
    double cx = mg->mx, cy = mg->my;

    double mdistsq = F_INF;
    for (vector<MapLine *>::iterator lit = mg->dll.begin(); lit != mg->dll.end(); lit++) {
        MapLine *line = *lit;
        MapNode *node = line->p1;
        double distsq = sq(cx - node->x) + sq(cy - node->y);
        if (distsq < mdistsq) { mdistsq = distsq; snode = node; }
        node = line->p2;
        distsq = sq(cx - node->x) + sq(cy - node->y);
        if (distsq < mdistsq) { mdistsq = distsq; snode = node; }
    }
}

void MapOperation::number_point()
{
    for (int num = 0; num < MAX_KBDNUM; num++)
        if (nnode[num] == snode)
            return;
    nnode[mg->kbd_num] = snode;
}

void MapOperation::number_way()
{
    //printf("NWAY %d\n", mg->kbd_num);
    for (int num = 0; num < MAX_KBDNUM; num++)
        if (nway[num] == sway)
            return;
    nway[mg->kbd_num] = sway;
}

void MapOperation::add_polyvertex() { pvl.push_back(MapPoint(mg->mx, mg->my)); }
void MapOperation::clear_polyvertex() { pvl.clear(); }

bool MapOperation::query_main_name(MapObject *ptr, wstring &name)
{
    for (vector<pair<string, wstring> >::iterator it = ptr->tl.begin(); it != ptr->tl.end(); it++)
        if (it->first.compare("name") == 0) {
            name = it->second;
            return true;
        }
    return false;
}
wstring MapOperation::get_node_string(MapNode *node)
{
    wstring ret, name;
    ret.append(s2ws(printf2str(" node %lld", node->id)));
    if (query_main_name((MapObject *) node, name)) ret.append(L" name " + name);
    //ret.append(s2ws(printf2str(" lat=%f lon=%f", node->lat, node->lon)));
    return ret;
}
wstring MapOperation::get_way_string(MapWay *way)
{
    wstring ret, name;
    ret.append(s2ws(printf2str(" way %lld", way->id)));
    if (query_main_name((MapObject *) way, name)) ret.append(L" name " + name);
    return ret;
}

MapNode *MapOperation::choose_nearest_sp_node() // choose nearest node which on shortest path
{
    MapPoint P(mg->mx, mg->my);
    MapNode *ret = NULL;
    double distsq = F_INF;
    for (vector<MapLine *>::iterator lit = mg->dll.begin(); lit != mg->dll.end(); lit++) {
        MapLine *line = *lit;
        MapPoint A(line->p1), B(line->p2);
        double adistsq = vlensq(A - P), bdistsq = vlensq(B - P);
        if (line->p1->on_shortest_path && adistsq < distsq) { ret = line->p1; distsq = adistsq; }
        if (line->p2->on_shortest_path && bdistsq < distsq) { ret = line->p2; distsq = bdistsq; }
    }
    return ret;
}

void MapOperation::set_shortestpath_start() { sp_start = choose_nearest_sp_node(); }
void MapOperation::set_shortestpath_end() { sp_end = choose_nearest_sp_node(); }

void MapOperation::clear_shortestpath_vertex() { sp_start = sp_end = NULL; }
void MapOperation::clear_shortestpath_result() { sp_result.clear(); sp_report.clear(); msp->prepare(); }
void MapOperation::show_shortestpath_result()
{
    if (!sp_result.empty()) {
        mgui->prepare_msgbox();
        mgui->set_msgbox_title(L"Shortest Path");
        mgui->set_msgbox_description(s2ws(printf2str("Here is the shortest path from node #%lld to node #%lld", sp_start->id, sp_end->id)));
        mgui->set_msgbox_append(L"[length] " + s2ws(md->get_length_string(sp_mindist)));
        mgui->set_msgbox_append(L"[algorithm] " + s2ws(msp->get_algo_name(sp_algo)));
        mgui->set_msgbox_append(s2ws(printf2str("[time] %.2f ms", sp_time)));
        mgui->set_msgbox_append(L"");
        mgui->set_msgbox_append(L"[source] " + get_node_string(sp_start));
        for (vector<MapLine *>::iterator lit = ++sp_result.begin(); lit != sp_result.end(); lit++) {
            MapLine *line = *lit;
            MapNode *node = line->p1;
            mgui->set_msgbox_append(L"-> " + get_node_string(node));
            mgui->set_msgbox_append(L"     ^ " + get_way_string(line->way));
        }
        mgui->set_msgbox_append(L"[destination] " + get_node_string(sp_end));
        mgui->show_msgbox();
    }
}

void MapOperation::run_shortestpath()
{
    if (!sp_start) { mg->msg.append("ERROR:\n  No source vertex."); return; }
    if (!sp_end) { mg->msg.append("ERROR:\n  No destination vertex."); return; }
    if (sp_start == sp_end) { mg->msg.append("ERROR:\n  Source and destination are same."); return; }
    
    msp->set_start(sp_start);
    msp->set_end(sp_end);
    sp_result.clear();
    if (!msp->get_shortest_path(sp_result, sp_mindist, sp_time, sp_prepare_time, sp_algo)) {
        mg->msg.append("ERROR:\n  Shortest path failed.");
        return;
    }
    sp_mindist *= MapData::dist_factor;
    sp_report = printf2str("Algorithm: %s\nDistance: %s\nInit: %.2f ms\nTime: %.2f ms\n",
        msp->get_algo_name(sp_algo), md->get_length_string(sp_mindist).c_str(), sp_prepare_time, sp_time);
    //show_shortestpath_result();
}

void MapOperation::switch_shortest_algo()
{
    int diff = mg->kbd_shift ? -1 : 1;
    printd("shift = %d, diff = %d\n", mg->kbd_shift, diff);
    int cnt = MapShortestPath::ALGO_COUNT;
    sp_algo = (sp_algo + diff + cnt) % cnt;
}

void MapOperation::clear_taxi_route() { tr.clear(); }
void MapOperation::show_taxi_route_begin() { if (!tr.empty()) mg->move_display_to_point(tr.front().x, tr.front().y); }
void MapOperation::show_taxi_route_end() { if (!tr.empty()) mg->move_display_to_point(tr.back().x, tr.back().y); }
void MapOperation::reload_taxi_route()
{
    tr.clear();
    for (vector<MapTaxiRoute::taxi_node>::iterator tnit = mtr->tnl.begin(); tnit != mtr->tnl.end(); tnit++) {
        double lat = tnit->lat;
        double lon = tnit->lon;
        double x, y;
        md->trans_coord(lat, lon, &x, &y);
        tr.push_back(MapPoint(x, y));
        //printd("%f %f\n", x, y);
    }
}
void MapOperation::select_taxi_route()
{
    mgui->prepare_inputbox();
    mgui->set_inputbox_title(L"Select taxi route");
    mgui->set_inputbox_description(L"Taxi ID");
    wstring uinput = mgui->show_inputbox();
    if (uinput.length() == 0) return;
    string str = ws2s(uinput);
    int taxi_id;
    
    if (sscanf(str.c_str(), "%d", &taxi_id) != 1) {
        mg->msg.append(printf2str("ERROR:\n  Invalid id '%s'", str.c_str()));
        return;
    }

    int taxi_index;
    taxi_index = mtr->find_index_by_id(taxi_id);
    if (taxi_index < 0) {
        mg->msg.append(printf2str("ERROR:\n  Can't find taxi %d\n", taxi_id));
        return;
    }
    
    mtr->load_route(taxi_index);
    reload_taxi_route();
}

void MapOperation::show_taxi_list()
{
    mgui->prepare_msgbox();
    mgui->set_msgbox_title(L"Taxi List");
    mgui->set_msgbox_description(L"Here is the taxi list");
    mgui->set_msgbox_append(s2ws(printf2str(" %-15s%-15s%-15s", "ID", "Node Count", "File Offset")));
    mgui->set_msgbox_append(L"==============================================");
    for (vector<pair<int, pair<int, long> > >::iterator tit = mtr->tl.begin(); tit != mtr->tl.end(); tit++) {
        int taxi_id = tit->first;
        int line_count = tit->second.first;
        long file_offset = tit->second.second;
        mgui->set_msgbox_append(s2ws(printf2str(" %-15d%-15d%-15ld", taxi_id, line_count, file_offset)));
    }
    mgui->show_msgbox();
}

void MapOperation::clear_all()
{
    clear_results();
    clear_select();
    clear_polyvertex();
    clear_shortestpath_vertex();
    clear_shortestpath_result();
    clear_taxi_route();
}

void MapOperation::init()
{
    sp_algo = 0;
    clear_all();
}

void MapOperation::operation(MapOperationCode op)
{
    assert(md);
    assert(mg);
    assert(mgui);
    mg->msg.clear();
    if (op == POP_DISPLAY) {
        mg->pop_display_range();
    } else {
        mg->push_display_range();
        switch (op) {
            case NOP: break;
            
            case MOVE_UP: mg->move_display_range(0, 1); break;
            case MOVE_DOWN: mg->move_display_range(0, -1); break;
            case MOVE_LEFT: mg->move_display_range(-1, 0); break;
            case MOVE_RIGHT: mg->move_display_range(1, 0); break;
            case ZOOM_OUT: mg->zoom_display_range(-1); break;
            case ZOOM_IN: mg->zoom_display_range(1); break;
            case ZOOM_OUT_BY_MOUSE: mg->zoom_display_range(-1, true); break;
            case ZOOM_IN_BY_MOUSE: mg->zoom_display_range(1, true); break;
            case RESET_VIEW: mg->reset_display_range(); break;
            case CENTER_NUM_POINT: mg->center_point(nnode[mg->kbd_num]); break;
            case CENTER_NUM_WAY: mg->center_way(nway[mg->kbd_num]); break;
            case CENTER_SEL_POINT: mg->center_point(snode); break;
            case CENTER_SEL_WAY: mg->center_way(sway); break;

            case QUERY_NAME: query_name(); break;
            case SELECT_WAY: select_way(); break;
            case SELECT_POINT: select_point(); break;
            case NUMBER_POINT: number_point(); break;
            case NUMBER_WAY: number_way(); break;
            case CLEAR_ALL: clear_all(); break;
            case SHOW_NODEINFO: show_nodeinfo(); break;
            case SHOW_WAYINFO: show_wayinfo(); break;
            case SHOW_QUERY_RESULT: show_results(); break;
            case ADD_POLYVERTEX: add_polyvertex(); break;
            case CLEAR_POLYVERTEX: clear_polyvertex(); break;
            case QUERY_TAG: query_tag(); break;
            case SET_SHORTESTPATH_START: set_shortestpath_start(); break;
            case SET_SHORTESTPATH_END: set_shortestpath_end(); break;
            case RUN_SHORTESTPATH: run_shortestpath(); break;
            case SHOW_SHORTESTPATH_RESULT: show_shortestpath_result(); break;
            case CLEAR_SHORTESTPATH: clear_shortestpath_vertex(); clear_shortestpath_result(); break;
            case SWITCH_SHORTESTPATH_ALGO: switch_shortest_algo(); break;
            case SELECT_TAXI_ROUTE: select_taxi_route(); break;
            case SHOW_TAXI_LIST: show_taxi_list(); break;
            case SHOW_TAXI_ROUTE_BEGIN: show_taxi_route_begin(); break;
            case SHOW_TAXI_ROUTE_END: show_taxi_route_end(); break;

            default: assert(0); break;
        }
    }
}

