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
    mg->msg += printf2str("Query Time = %.2f ms\n", qtime);
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
    string tag = ws2s(wstr);
    
    assert(pvl.size() >= 3);
    MapRect base = pvl.front().get_rect();
    for (vector<MapPoint>::iterator it = ++pvl.begin(); it != pvl.end(); it++)
        base.merge(it->get_rect());
    
    query_tag_with_filter(tag, base, &MapOperation::poly_node_filter, &MapOperation::poly_way_filter);
    
    select_results();
    show_results();
}

MapPoint MapOperation::nearby_center;

bool MapOperation::dist_node_filter(MapNode *node) { return lensq(MapPoint(node->x, node->y) - nearby_center) <= nearby_distsq; }
bool MapOperation::dist_way_filter(MapWay *way)
{
    for (vector<MapNode *>::iterator nit = way->nl[0].begin(); nit != way->nl[0].end(); nit++) {
        MapNode *node = *nit;
        if (lensq(MapPoint(node->x, node->y) - nearby_center) <= nearby_distsq)
            return true;
    }
    return false;
}

bool MapOperation::sort_node_by_dist(MapNode *a, MapNode *b)
{
    return lensq(MapPoint(a->x, a->y) - nearby_center) < lensq(MapPoint(b->x, b->y) - nearby_center);
}

bool MapOperation::sort_way_by_dist(MapWay *a, MapWay *b)
{
    double alensq = F_INF, blensq = F_INF;
    for (vector<MapNode *>::iterator nit = a->nl[0].begin(); nit != a->nl[0].end(); nit++) {
        MapNode *node = *nit;
        alensq = min(alensq, lensq(MapPoint(node->x, node->y) - nearby_center));
    }
    for (vector<MapNode *>::iterator nit = b->nl[0].begin(); nit != b->nl[0].end(); nit++) {
        MapNode *node = *nit;
        blensq = min(blensq, lensq(MapPoint(node->x, node->y) - nearby_center));
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
    string tag = ws2s(wstr);
    
    assert(pvl.size() == 2);
    MapRect base = pvl.front().get_rect();
    MapPoint A(pvl.front().x, pvl.front().y);
    MapPoint B(pvl.back().x, pvl.back().y);
    nearby_center = A;
    nearby_distsq = lensq(B - A);
    double nearby_dist = sqrt(nearby_distsq);
    base.left -= nearby_dist;
    base.right += nearby_dist;
    base.top += nearby_dist;
    base.bottom -= nearby_dist;
    
    query_tag_with_filter(tag, base, &MapOperation::dist_node_filter, &MapOperation::dist_way_filter);
    
    sort(nresult.begin(), nresult.end(), sort_node_by_dist);
    sort(wresult.begin(), wresult.end(), sort_way_by_dist);
    
    select_results();
    show_results();
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
    string tag = ws2s(wstr);
    
    double dminx, dminy, dmaxx, dmaxy;
    mg->get_display_range(&dminx, &dmaxx, &dminy, &dmaxy);
    MapRect base = MapRect(dminx, dmaxx, dminy, dmaxy);
    
    query_tag_with_filter(tag, base, &MapOperation::true_node_filter, &MapOperation::true_way_filter);
    
    select_results();
    show_results();
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
//    mg->msg.append("Hello World!\n");
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
            case QUERY_TAG: query_tag(); break;
            default: assert(0); break;
        }
    }
}

