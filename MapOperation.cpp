#include "common.h"
#include "MapOperation.h"
#include "MapGraphics.h"
#include "MapVector.h"
#include "wstr.h"

using namespace std;

void MapOperation::query_name()
{
    mgui->prepare_inputbox();
    mgui->set_inputbox_title(L"Query objects by name");
    mgui->set_inputbox_description(L"Query Name");
    wstring uinput = mgui->show_inputbox();
    if (uinput.length() == 0) return;
    const wchar_t *wstr = uinput.c_str();
    query_description = L"Query by name: " + uinput;
    
    nresult.clear();
    wresult.clear();
    TIMING ("query by name", {
        md->nd.find(nresult, wstr);
        md->wd.find(wresult, wstr);
    })
    for (int i = 0; i < MAX_KBDNUM; i++) {
        if (i < (LL) nresult.size()) nnode[i] = nresult[i];
        if (i < (LL) wresult.size()) nway[i] = wresult[i];
    }
    
    show_results();
}

void MapOperation::show_results()
{
    char buf[MAXLINE];
    mgui->prepare_msgbox();
    mgui->set_msgbox_title(L"Query esult");
    mgui->set_msgbox_description(L"Here are query results:");
    mgui->set_msgbox_append(L"== Query ==");
    mgui->set_msgbox_append(s2ws("  ") + query_description);
    mgui->set_msgbox_append(L"");
    
    for (vector<MapNode *>::iterator it = nresult.begin(); it != nresult.end(); it++) {
        MapNode *node = *it;
        sprintf(buf, "== Node Result %d : #%lld ==", (int)(it - nresult.begin()), node->id);
        mgui->set_msgbox_append(s2ws(string(buf)));
        for (map<string, const wchar_t *>::iterator it = node->names.begin(); it != node->names.end(); it++) {
            mgui->set_msgbox_append(L"  [" + s2ws(it->first) + L"] " + wstring(it->second));
        }
    }
    mgui->set_msgbox_append(L"");
    mgui->set_msgbox_append(L"");
    
    for (vector<MapWay *>::iterator it = wresult.begin(); it != wresult.end(); it++) {
        MapWay *way = *it;
        sprintf(buf, "== Way Result %d : #%lld ==", (int)(it - wresult.begin()), way->id);
        mgui->set_msgbox_append(s2ws(string(buf)));
        for (map<string, const wchar_t *>::iterator it = way->names.begin(); it != way->names.end(); it++) {
            mgui->set_msgbox_append(L"  [" + s2ws(it->first) + L"] " + wstring(it->second));
        }
    }
    
    mgui->show_msgbox();
}

void MapOperation::show_wayinfo()
{
    if (sway) {
        char buf[MAXLINE];
        mgui->prepare_msgbox();
        mgui->set_msgbox_title(s2ws("Way Information"));
        sprintf(buf, "Here is information about way %lld", sway->id);
        mgui->set_msgbox_description(s2ws(string(buf)));
        
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
        if (way_length >= 1e3)
            sprintf(buf, "[length] %.2f km", way_length * 1e-3);
        else
            sprintf(buf, "[length] %.2f m", way_length);
        mgui->set_msgbox_append(s2ws(string(buf)));
        
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
            if (way_area >= 1e3)
                sprintf(buf, "[area] %.3f km2", way_area * 1e-6);
            else
                sprintf(buf, "[area] %.2f m2", way_area);
            mgui->set_msgbox_append(s2ws(string(buf)));
        }
        mgui->show_msgbox();
    }
}

void MapOperation::show_nodeinfo()
{
    if (snode) {
        char buf[MAXLINE];
        mgui->prepare_msgbox();
        mgui->set_msgbox_title(s2ws("Node Information"));
        sprintf(buf, "Here is information about node %lld", snode->id);
        mgui->set_msgbox_description(s2ws(string(buf)));
        
        // node names
        for (map<string, const wchar_t *>::iterator it = snode->names.begin(); it != snode->names.end(); it++) {
            mgui->set_msgbox_append(L"[" + s2ws(it->first) + L"] " + wstring(it->second));
        }
        
        // node coord
        sprintf(buf, "[coord] lat=%f lon=%f", snode->lat, snode->lon);
        mgui->set_msgbox_append(s2ws(string(buf)));
        
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

void MapOperation::operation(MapOperationCode op)
{
    assert(md);
    assert(mg);
    assert(mgui);
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
            case CLEAR_SELECT: clear_select(); break;
            case SHOW_NODEINFO: show_nodeinfo(); break;
            case SHOW_WAYINFO: show_wayinfo(); break;
            case SHOW_QUERY_RESULT: show_results(); break;
            default: assert(0); break;
        }
    }
}

