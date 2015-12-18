#include <cstdio>
#include "common.h"
#include "MapOperation.h"
#include "MapGraphics.h"
#include "MapVector.h"
#include "wstr.h"
#include "printf2str.h"

using namespace std;
void MapOperation::query_timer_start() { qclock = clock(); }
void MapOperation::query_timer_stop()
{
    qtime = (double) (clock() - qclock) / CLOCKS_PER_SEC * 1000;
    mg->msg += printf2str("Query Time = %.2f ms\n", qtime);
}

void MapOperation::query_tag_with_filter(const string &tag, const MapRect &baserect,
          bool (MapOperation::*node_filter)(MapNode *),
          bool (MapOperation::*way_filter)(MapWay *))
{
    clear_results();
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
    } else {
        mg->msg.append(printf2str("ERROR:\n  Invalid tag \"%s\".\n", tag.c_str()));
        return;
    }
    
    select_results();
    show_results();
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
    if (pvl.size() < 3) {
        mg->msg.append("ERROR:\n  No polygon selected.\n");
        return;
    }
    mgui->prepare_inputbox();
    mgui->set_inputbox_title(L"Query objects in polygon by tag");
    mgui->set_inputbox_description(L"Tag Name");
    wstring uinput = mgui->show_inputbox();
    if (uinput.length() == 0) return;
    const wchar_t *wstr = uinput.c_str();
    query_description = L"  Query in polygon by tag: " + uinput;
    string tag = ws2s(wstr);
    
    assert(pvl.size() >= 3);
    MapRect base = pvl.front().get_rect();
    for (vector<MapPoint>::iterator it = ++pvl.begin(); it != pvl.end(); it++)
        base.merge(it->get_rect());
    
    query_tag_with_filter(tag, base, &MapOperation::poly_node_filter, &MapOperation::poly_way_filter);
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
    
    query_timer_start();
    md->nd.find(nresult, wstr);
    md->wd.find(wresult, wstr);
    query_timer_stop();
    
    select_results();
    show_results();
}

void MapOperation::select_results()
{
    clear_select();
    for (int i = 0; i < MAX_KBDNUM; i++) {
        if (i < (LL) nresult.size()) nnode[i] = nresult[i];
        if (i < (LL) wresult.size()) nway[i] = wresult[i];
    }
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
        for (map<string, const wchar_t *>::iterator it = node->names.begin(); it != node->names.end(); it++) {
            mgui->set_msgbox_append(L"  [" + s2ws(it->first) + L"] " + wstring(it->second));
        }
    }
    mgui->set_msgbox_append(L"");
    mgui->set_msgbox_append(L"");
    
    for (vector<MapWay *>::iterator it = wresult.begin(); it != wresult.end(); it++) {
        MapWay *way = *it;
        mgui->set_msgbox_append(s2ws(printf2str("== Way Result %d : #%lld ==", (int)(it - wresult.begin()), way->id)));
        for (map<string, const wchar_t *>::iterator it = way->names.begin(); it != way->names.end(); it++) {
            mgui->set_msgbox_append(L"  [" + s2ws(it->first) + L"] " + wstring(it->second));
        }
    }
    
    mgui->show_msgbox();
}

void MapOperation::clear_results()
{
    nresult.clear();
    wresult.clear();
    query_description = L"  No Query";
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
        for (map<string, const wchar_t *>::iterator it = sway->names.begin(); it != sway->names.end(); it++) {
            mgui->set_msgbox_append(L"[" + s2ws(it->first) + L"] " + wstring(it->second));
        }
        
        // way length
        double way_length = 0;
        MapPoint A(sway->nl[0].front()->x, sway->nl[0].front()->y), B;
        for (vector<MapNode *>::iterator it = ++sway->nl[0].begin(); it != sway->nl[0].end(); it++) {
            MapNode *node = *it;
            B = MapPoint(node->x, node->y);
            if (it != sway->nl[0].begin()) {
                way_length += len(B - A);
            }
            A = B;
        }
        way_length *= dist_factor;
        string lstr;
        if (way_length >= 1e3)
            lstr = printf2str("[length] %.2f km", way_length * 1e-3);
        else
            lstr = printf2str("[length] %.2f m", way_length);
        mgui->set_msgbox_append(s2ws(lstr));
        
        // way area
        if (!sway->nl[0].empty() && sway->nl[0].front() == sway->nl[0].back()) {
            double way_area = 0;
            MapPoint P(sway->nl[0].front()->x, sway->nl[0].front()->y);
            MapPoint A(P), B;
            for (vector<MapNode *>::iterator it = ++sway->nl[0].begin(); it != sway->nl[0].end(); it++) {
                MapNode *node = *it;
                B = MapPoint(node->x, node->y);
                way_area += det(B - A, B - P);
                A = B;
            }
            way_area = fabs(way_area * sq(dist_factor) / 2);
            string astr;
            if (way_area >= 1e3)
                astr = printf2str("[area] %.3f km2", way_area * 1e-6);
            else
                astr = printf2str("[area] %.2f m2", way_area);
            mgui->set_msgbox_append(s2ws(astr));
        }
        
        // way tags
        for (vector<wstring>::iterator it = sway->taglist.begin(); it != sway->taglist.end(); it++) {
            mgui->set_msgbox_append(L"[tag] " + *it);
        }
        mgui->show_msgbox();
    }
}

void MapOperation::show_nodeinfo()
{
    if (snode) {
        mgui->prepare_msgbox();
        mgui->set_msgbox_title(s2ws("Node Information"));
        mgui->set_msgbox_description(s2ws(printf2str("Here is information about node #%lld", snode->id)));
        mgui->set_msgbox_append(s2ws(printf2str("[id] #%lld", snode->id)));
        
        // node names
        for (map<string, const wchar_t *>::iterator it = snode->names.begin(); it != snode->names.end(); it++) {
            mgui->set_msgbox_append(L"[" + s2ws(it->first) + L"] " + wstring(it->second));
        }
        
        // node coord
        mgui->set_msgbox_append(s2ws(printf2str("[coord] lat=%f lon=%f", snode->lat, snode->lon)));
        
        // node tags
        for (vector<wstring>::iterator it = snode->taglist.begin(); it != snode->taglist.end(); it++) {
            mgui->set_msgbox_append(L"[tag] " + *it);
        }
        
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
    for (vector<MapWay *>::iterator wit = mg->dwl.begin(); wit != mg->dwl.end(); wit++) {
        MapWay *way = *wit;
        MapPoint A, B;
        for (vector<MapNode *>::iterator nit = way->nl[mg->clvl].begin(); nit != way->nl[mg->clvl].end(); nit++) {
            MapNode *node = *nit;
            B = MapPoint(node->x, node->y);
            if (nit != way->nl[mg->clvl].begin()) {
                double distsq = distsq_p2s(P, A, B);
                if (distsq < mdistsq) { mdistsq = distsq; sway = way; }
            }
            A = B;
        }
    }
}


void MapOperation::select_point()
{
    double cx = mg->mx, cy = mg->my;

    double mdistsq = F_INF;
    for (vector<MapWay *>::iterator wit = mg->dwl.begin(); wit != mg->dwl.end(); wit++) {
        MapWay *way = *wit;
        for (vector<MapNode *>::iterator nit = way->nl[mg->clvl].begin(); nit != way->nl[mg->clvl].end(); nit++) {
            MapNode *node = *nit;
            double distsq = sq(cx - node->x) + sq(cy - node->y);
            if (distsq < mdistsq) { mdistsq = distsq; snode = node; }
        }
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

void MapOperation::clear_all()
{
    clear_results();
    clear_select();
    clear_polyvertex();
}

void MapOperation::operation(MapOperationCode op)
{
    assert(md);
    assert(mg);
    assert(mgui);
    mg->msg.clear();
    mg->msg.append("Hello World!\n");
    if (op == POP_DISPLAY) {
        mg->pop_display_range();
    } else {
        mg->push_display_range();
        switch (op) {
            case MOVE_UP: mg->move_display_range(0, 1); break;
            case MOVE_DOWN: mg->move_display_range(0, -1); break;
            case MOVE_LEFT: mg->move_display_range(-1, 0); break;
            case MOVE_RIGHT: mg->move_display_range(1, 0); break;
            case ZOOM_OUT: mg->zoom_display_range(-1); break;
            case ZOOM_IN: mg->zoom_display_range(1); break;
            case RESET_VIEW: mg->reset_display_range(); break;
            case TOGGLE_RTREE: mg->show_rtree ^= 1; break;
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
            case QUERY_TAG_WITH_POLY: query_tag_with_poly(); break;
            default: assert(0); break;
        }
    }
}

