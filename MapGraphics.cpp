#include <cmath>
#include <cstdio>
#include <cstring>
#include <GL/glut.h>
#include "common.h"
#include "MapData.h"
#include "MapGraphics.h"
#include "MapRect.h"
#include "MapGUI.h"
#include "wstr.h"
#include "MapVector.h"

#include <algorithm>
#include <set>

using namespace std;

void MapGraphics::target(MapData *md) { MapGraphics::md = md; }
void MapGraphics::target_gui(MapGUI *mgui) { MapGraphics::mgui = mgui; }

// gui staff
void MapGraphics::query_name()
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
    for (int i = 0; i < NUM_MAX; i++) {
        if (i < (LL) nresult.size()) nnode[i] = nresult[i];
        if (i < (LL) wresult.size()) nway[i] = wresult[i];
    }
    
    show_results();
}

void MapGraphics::show_results()
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

void MapGraphics::show_wayinfo()
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
        MapPoint A, B;
        for (vector<MapNode *>::iterator it = sway->nl[0].begin(); it != sway->nl[0].end(); it++) {
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

void MapGraphics::show_nodeinfo()
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

void MapGraphics::clear_select()
{
    snode = NULL;
    sway = NULL;
    memset(nnode, 0, sizeof(nnode));
    memset(nway, 0, sizeof(nway));
}

void MapGraphics::select_way()
{
    //printf("mx=%f my=%f\n", mx, my);
    double cx, cy; // cursor, map coord
    rtrans_gcoord(mx, my, &cx, &cy);
    /*double mdist = -1;
    MapPoint P(cx, cy);
    for (vector<MapLine *>::iterator lit = dll.begin(); lit != dll.end(); lit++) {
        MapLine *line = *lit;
        MapPoint A(line->p1->x, line->p1->y), B(line->p2->x, line->p2->y);
        double dist = dts(P, A, B); 
        if (mdist < 0 || dist < mdist) { mdist = dist; sway = line->way; }
    }*/

    double mdistsq = F_INF;
    MapPoint P(cx, cy);
    for (vector<MapWay *>::iterator wit = dwl.begin(); wit != dwl.end(); wit++) {
        MapWay *way = *wit;
        MapPoint A, B;
        for (vector<MapNode *>::iterator nit = way->nl[0].begin(); nit != way->nl[0].end(); nit++) {
            MapNode *node = *nit;
            B = MapPoint(node->x, node->y);
            if (nit != way->nl[0].begin()) {
                double distsq = distsq_p2s(P, A, B);
                if (distsq < mdistsq) { mdistsq = distsq; sway = way; }
            }
            A = B;
        }
    }
}


void MapGraphics::select_point()
{
    double cx, cy; // cursor, map coord
    rtrans_gcoord(mx, my, &cx, &cy);

    double mdistsq = F_INF;
    for (vector<MapWay *>::iterator wit = dwl.begin(); wit != dwl.end(); wit++) {
        MapWay *way = *wit;
        for (vector<MapNode *>::iterator nit = way->nl[0].begin(); nit != way->nl[0].end(); nit++) {
            MapNode *node = *nit;
            double distsq = sq(cx - node->x) + sq(cy - node->y);
            if (distsq < mdistsq) { mdistsq = distsq; snode = node; }
        }
    }
}

void MapGraphics::number_point()
{
    for (int num = 0; num < NUM_MAX; num++)
        if (nnode[num] == snode)
            return;
    nnode[kbd_num] = snode;
}

void MapGraphics::number_way()
{
    //printf("NWAY %d\n", kbd_num);
    for (int num = 0; num < NUM_MAX; num++)
        if (nway[num] == sway)
            return;
    nway[kbd_num] = sway;
}

// graphics staff
double MapGraphics::get_display_resolution()
{
    return (md->maxy - md->miny) * pow(zoom_step, zoom_level);
}

void MapGraphics::trans_gcoord(double x, double y, double *gx, double *gy)
{
    *gx = x - md->minx;
    *gy = y - md->miny;
}

void MapGraphics::rtrans_gcoord(double gx, double gy, double *x, double *y)
{
    *x = gx + md->minx;
    *y = gy + md->miny;
#ifdef DEBUG
    double gx2, gy2;
    trans_gcoord(*x, *y, &gx2, &gy2);
    assert(fequ(gx2, gx) && fequ(gy2, gy));
#endif
}


void MapGraphics::set_display_range(double dminx, double dmaxx, double dminy, double dmaxy)
{
    assert(dminx < dmaxx); assert(dminy < dmaxy);
    trans_gcoord(dminx, dminy, &(MapGraphics::dminx), &(MapGraphics::dminy));  // set limit for MapGraphics
    trans_gcoord(dmaxx, dmaxy, &(MapGraphics::dmaxx), &(MapGraphics::dmaxy));
    zoom_level = 0;
    display_stack.clear();
}

void MapGraphics::push_display_range()
{
    double centerx = dminx + (dmaxx - dminx) / 2;
    double centery = dminy + (dmaxy - dminy) / 2;
    if (display_stack.empty() ||
            (!fequ(display_stack.back().first.first, centerx) ||
             !fequ(display_stack.back().first.second, centery) ||
             display_stack.back().second != zoom_level)) {
        //printf("PUSH!\n");
        display_stack.push_back(make_pair(make_pair(centerx, centery), zoom_level));
    }
}

void MapGraphics::pop_display_range()
{
    if (!display_stack.empty()) {
        //printf("POP!\n");
        double centerx, centery;
        int f;
        centerx = display_stack.back().first.first;
        centery = display_stack.back().first.second;
        f = display_stack.back().second;
        display_stack.pop_back();
        move_display_to_point(centerx, centery);
        zoom_display_range(f - zoom_level);
    }
}

void MapGraphics::move_display_range(int x, int y)
{
    double mdiff = min(dmaxx - dminx, dmaxy - dminy);
    if (x != 0) {
        double diffx = mdiff * move_step * x;
        dmaxx += diffx;
        dminx += diffx;
    }
    if (y != 0) {
        double diffy = mdiff * move_step * y;
        dmaxy += diffy;
        dminy += diffy;
    }
}

void MapGraphics::zoom_display_range(int f)
{
    double diffx = (dmaxx - dminx) * (1 - pow(zoom_step, f));
    dmaxx -= diffx / 2;
    dminx += diffx / 2;
    
    double diffy = (dmaxy - dminy) * (1 - pow(zoom_step, f));
    dmaxy -= diffy / 2;
    dminy += diffy / 2;
    
    zoom_level += f;
}


void MapGraphics::zoom_display_by_size(double sizex, double sizey)
{
    sizex /= zoom_bysize_factor;
    sizey /= zoom_bysize_factor;
    
    //             sizex < smallerx   --> make diffx smaller
    //  diffx > sizex >= smallerx
    //  diffx <= sizex                --> make diffx bigger
    
    const int F_LIMIT = 100;
    double diffx = dmaxx - dminx;
    int xf = 0;
    while (abs(xf) <= F_LIMIT && diffx <= sizex) { diffx *= pow(zoom_step, -1); xf--; }
    while (abs(xf) <= F_LIMIT && sizex < diffx) { diffx *= pow(zoom_step, 1); xf++; }
    
    double diffy = dmaxy - dminy;
    int yf = 0;
    while (abs(xf) <= F_LIMIT && diffy <= sizey) { diffy *= pow(zoom_step, -1); yf--; }
    while (abs(xf) <= F_LIMIT && sizey < diffy) { diffy *= pow(zoom_step, 1); yf++; }
    
    int f = min(xf, yf);
    zoom_display_range(f);
}

void MapGraphics::move_display_to_point(double gx, double gy)
{
    double diffx = (dmaxx - dminx);
    dmaxx = gx + diffx / 2;
    dminx = gx - diffx / 2;
    
    double diffy = (dmaxy - dminy);
    dmaxy = gy + diffy / 2;
    dminy = gy - diffy / 2;
}

void MapGraphics::reset_display_range()
{
    set_display_range(md->minx, md->maxx, md->miny, md->maxy);
    
    double init_width = initial_window_height * md->map_ratio;
    double init_height = initial_window_height;
    
    // do something like reshape()
    double old_diffx = dmaxx - dminx;
    double old_diffy = dmaxy - dminy;
    double new_diffx = old_diffx / init_width * window_width;
    double new_diffy = old_diffy / init_height * window_height;
    
    dmaxx = dmaxx + (new_diffx - old_diffx) / 2;
    dminx = dminx - (new_diffx - old_diffx) / 2;
    dmaxy = dmaxy + (new_diffy - old_diffy) / 2;
    dminy = dminy - (new_diffy - old_diffy) / 2;
}

void MapGraphics::center_point(MapNode *node)
{
    if (node) {
        snode = node;
        double gx, gy;
        trans_gcoord(node->x, node->y, &gx, &gy);
        move_display_to_point(gx, gy);
        /*MapRect rect = node->way->get_rect();
        MapRect grect;
        trans_gcoord(rect.left, rect.bottom, &grect.left, &grect.bottom);
        trans_gcoord(rect.right, rect.top, &grect.right, &grect.top);
        zoom_display_by_size(grect.right - grect.left, grect.top - grect.bottom);*/
    }
}

void MapGraphics::center_way(MapWay *way)
{
    if (way) {
        sway = way;
        MapRect rect = way->get_rect();
        MapRect grect;
        trans_gcoord(rect.left, rect.bottom, &grect.left, &grect.bottom);
        trans_gcoord(rect.right, rect.top, &grect.right, &grect.top);
        double gx, gy;
        gx = (grect.left + grect.right) / 2;
        gy = (grect.bottom + grect.top) / 2;
        move_display_to_point(gx, gy);
        zoom_display_by_size(grect.right - grect.left, grect.top - grect.bottom);
    }
}

void MapGraphics::map_operation(MapGraphicsOperation op)
{
    if (op == POP_DISPLAY) {
        pop_display_range();
    } else {
        push_display_range();
        switch (op) {
            case UP: move_display_range(0, 1); break;
            case DOWN: move_display_range(0, -1); break;
            case LEFT: move_display_range(-1, 0); break;
            case RIGHT: move_display_range(1, 0); break;
            case ZOOM_OUT: zoom_display_range(-1); break;
            case ZOOM_IN: zoom_display_range(1); break;
            case RESET_VIEW: reset_display_range(); break;
            case TOGGLE_RTREE: show_rtree ^= 1; break;
            case QUERY_NAME: query_name(); break;
            case SELECT_WAY: select_way(); break;
            case SELECT_POINT: select_point(); break;
            case NUMBER_POINT: number_point(); break;
            case NUMBER_WAY: number_way(); break;
            case CLEAR_SELECT: clear_select(); break;
            case CENTER_NUM_POINT: center_point(nnode[kbd_num]); break;
            case CENTER_NUM_WAY: center_way(nway[kbd_num]); break;
            case CENTER_SEL_POINT: center_point(snode); break;
            case CENTER_SEL_WAY: center_way(sway); break;
            case SHOW_NODEINFO: show_nodeinfo(); break;
            case SHOW_WAYINFO: show_wayinfo(); break;
            case SHOW_QUERY_RESULT: show_results(); break;
            default: assert(0); break;
        }
    }
    glutPostRedisplay();
}

static MapGraphics *mgptr = NULL;


void MapGraphics::highlight_point(MapNode *node, float color[], float thick)
{
    double gx, gy;
    trans_gcoord(node->x, node->y, &gx, &gy);
    double diff = selected_point_rect_size / 2.0 * (dmaxx - dminx) / window_width;
    glColor3f(color[0], color[1], color[2]);
    glLineWidth(thick);
    glPointSize(thick);
    glBegin(GL_LINE_LOOP);
    glVertex2d(gx - diff, gy - diff);
    glVertex2d(gx - diff, gy + diff);
    glVertex2d(gx + diff, gy + diff);
    glVertex2d(gx + diff, gy - diff);
    glEnd();
    glBegin(GL_POINTS);
    glVertex2d(gx, gy);
    glEnd();
}

void MapGraphics::draw_way(MapWay *way)
{
    glBegin(GL_LINE_STRIP);
    for (vector<MapNode *>::iterator nit = way->nl[0].begin(); nit != way->nl[0].end(); nit++) {
        MapNode *node = *nit;
        double gx, gy;
        trans_gcoord(node->x, node->y, &gx, &gy);
        glVertex2d(gx, gy);
    }
    glEnd();
}

void MapGraphics::redraw()
{
    //fflush(stdout);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(dminx, dmaxx, dminy, dmaxy);
//    printf("range: %f %f %f %f\n", dminx, dmaxx, dminy, dmaxy);
//    printf("disp-res: %f\n", get_display_resolution());
//    printf("zoom-level: %d\n", zoom_level);
    
    double mminx, mmaxx, mminy, mmaxy;
    rtrans_gcoord(dminx, dminy, &mminx, &mminy);
    rtrans_gcoord(dmaxx, dmaxy, &mmaxx, &mmaxy);
    
    dll.clear();
    int lvl_low_limit = md->ml.select_level(get_display_resolution());
    printf("display level: %d\n", lvl_low_limit);
    md->lrt[lvl_low_limit].find(dll, MapRect(mminx, mmaxx, mminy, mmaxy));
    printf("r-tree result lines: %lld\n", (LL) dll.size());

    // draw r-tree rectangles

    if (show_rtree) {
        vector<MapRect> rtree_rects;
        md->lrt[lvl_low_limit].get_all_tree_rect(rtree_rects);
        printf("rect count = %lld\n", (LL) rtree_rects.size());
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.0f);
        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        for (vector<MapRect>::iterator rit = rtree_rects.begin(); rit != rtree_rects.end(); rit++) {
            MapRect &rect = *rit;
            trans_gcoord(rect.left, rect.bottom, &rect.left, &rect.bottom);
            trans_gcoord(rect.right, rect.top, &rect.right, &rect.top);
            glVertex2d(rect.left, rect.bottom);
            glVertex2d(rect.right, rect.bottom);
            glVertex2d(rect.right, rect.top);
            glVertex2d(rect.left, rect.top);
        }
        glEnd();
    }

    
    
    
    
    dwl.clear();
    for (vector<MapLine *>::iterator lit = dll.begin(); lit != dll.end(); lit++)
        dwl.push_back((*lit)->way);
    sort(dwl.begin(), dwl.end());
    dwl.resize(unique(dwl.begin(), dwl.end()) - dwl.begin());
    
    /*dwl.clear();
    set<MapWay *> wayset;
    for (vector<MapLine *>::iterator lit = dll.begin(); lit != dll.end(); lit++) {
        MapWay *way = (*lit)->way;
        if (wayset.insert(way).second) dwl.push_back(way);
    }*/
    printf("need to draw %lld ways\n", (LL) dwl.size());
    
    bool sway_flag = false;
    for (int num = 0; num < NUM_MAX; num++) {
        MapWay *way = nway[num];
        if (way) {
            if (way == sway) sway_flag = true;
            glColor3f(ncolor[num][0], ncolor[num][1], ncolor[num][2]);
            glLineWidth(selected_way_thick);
            draw_way(way);
        }
    }
    if (sway && !sway_flag) {
        glColor3f(scolor[0], scolor[1], scolor[2]);
        glLineWidth(selected_way_thick);
        draw_way(sway);
    }
    for (vector<MapWay *>::iterator wit = dwl.begin(); wit != dwl.end(); wit++) {
        MapWay *way = *wit;
        float r, g, b, thickness;
        md->wt.query_rgbt(way->waytype, &r, &g, &b, &thickness);
        glColor3f(r, g, b);
        glLineWidth(thickness);
        draw_way(way);
    }
    
    bool snode_flag = false;
    for (int num = 0; num < NUM_MAX; num++) {
        MapNode *node = nnode[num];
        if (node) {
            highlight_point(node, ncolor[num], selected_point_rect_thick);
            if (node == snode) snode_flag = true;
        }
    }
    if (snode && !snode_flag)
        highlight_point(snode, scolor, selected_point_rect_thick);
    
    glFlush();
}

void MapGraphics::special_keyevent(int key, int x, int y)
{
    MapGraphicsOperation op;
    switch (key) {
        case GLUT_KEY_F1: op = RESET_VIEW; break;
        case GLUT_KEY_F2: op = TOGGLE_RTREE; break;
        case GLUT_KEY_F3: op = QUERY_NAME; break;
        case GLUT_KEY_F4: op = CLEAR_SELECT; break;
        case GLUT_KEY_F8: op = SHOW_QUERY_RESULT; break;
        case GLUT_KEY_F9: op = CENTER_SEL_POINT; break;
        case GLUT_KEY_F10: op = CENTER_SEL_WAY; break;
        case GLUT_KEY_F11: op = SHOW_NODEINFO; break;
        case GLUT_KEY_F12: op = SHOW_WAYINFO; break;
        case GLUT_KEY_UP: op = UP; break;
        case GLUT_KEY_DOWN: op = DOWN; break;
        case GLUT_KEY_LEFT: op = LEFT; break;
        case GLUT_KEY_RIGHT: op = RIGHT; break;
        case GLUT_KEY_PAGE_UP: op = ZOOM_OUT; break;
        case GLUT_KEY_PAGE_DOWN: op = ZOOM_IN; break;
        default: printf("unknown key %d\n", key); return;
    }
    map_operation(op);
}

void MapGraphics::keyevent(unsigned char key, int x, int y)
{
    MapGraphicsOperation op;
    char kbd_char = key;
    if (isupper(kbd_char)) kbd_char = kbd_char - 'A' + 'a';
    int shift = 0;
    switch (kbd_char) { // convert SHIFT+NUM to NUM
        case '!': kbd_char = '1'; shift = 1; break;
        case '@': kbd_char = '2'; shift = 1; break;
        case '#': kbd_char = '3'; shift = 1; break;
        case '$': kbd_char = '4'; shift = 1; break;
        case '%': kbd_char = '5'; shift = 1; break;
        case '^': kbd_char = '6'; shift = 1; break;
        case '&': kbd_char = '7'; shift = 1; break;
        case '*': kbd_char = '8'; shift = 1; break;
        case '(': kbd_char = '9'; shift = 1; break;
        case ')': kbd_char = '0'; shift = 1; break;
    }
    if (isdigit(kbd_char)) {
        kbd_num = kbd_char == '0' ? 9 : kbd_char - '1';
        assert(kbd_num < NUM_MAX);
        if (shift)
            op = NUMBER_POINT;
        else
            op = CENTER_NUM_POINT;
    } else if (strchr("qwertyuiop", kbd_char)) {
        const char *s = "qwertyuiop";
        kbd_num = strchr(s, kbd_char) - s;
        assert(kbd_num < NUM_MAX);
        if (shift)
            op = NUMBER_WAY;
        else
            op = CENTER_NUM_WAY;
    } else switch (kbd_char) {
        case '-': case '_' : op = ZOOM_OUT; break;
        case '+': case '=' : op = ZOOM_IN; break;
        case 'k': op = UP; break;
        case 'j': op = DOWN; break;
        case 'h': op = LEFT; break;
        case 'l': op = RIGHT; break;
        case '\x1b': op = POP_DISPLAY; break; // ESC
        default: printf("unknown key %c\n", key); return;
    }
    map_operation(op);
}

void MapGraphics::set_mouse_coord(int x, int y)
{
    mx = dminx + ((double) x / window_width) * (dmaxx - dminx);
    my = dminy + ((double) (window_height - y) / window_height) * (dmaxy - dminy);
}

void MapGraphics::mouse_event(bool use_last_op, int button, int state, int x, int y)
{
    MapGraphicsOperation op;
    if (use_last_op) {
        op = last_mouse_op;
    } else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        op = SELECT_WAY;
    } else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        op = SELECT_POINT;
    } else {
        printf("unknown mouse event %d %d\n", button, state);
        return;
    }
    last_mouse_op = op;
    set_mouse_coord(x, y);
    map_operation(op);
}

void MapGraphics::mouseupdown(int button, int state, int x, int y) { mouse_event(false, button, state, x, y); }
void MapGraphics::mousemotion(int x, int y) { mouse_event(true, -1, -1, x, y); }

void MapGraphics::reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    double old_diffx = dmaxx - dminx;
    double old_diffy = dmaxy - dminy;
    double new_diffx = old_diffx / window_width * width;
    double new_diffy = old_diffy / window_height * height;
    
    dmaxx = dmaxx + (new_diffx - old_diffx) / 2;
    dminx = dminx - (new_diffx - old_diffx) / 2;
    dmaxy = dmaxy + (new_diffy - old_diffy) / 2;
    dminy = dminy - (new_diffy - old_diffy) / 2;
    
    window_width = width;
    window_height = height;
    glutPostRedisplay();
}

//void redraw_wrapper() { assert(mgptr); mgptr->redraw(); }
static void redraw_wrapper() { assert(mgptr); TIMING("redraw", { mgptr->redraw(); }) }
static void reshape_wrapper(int width, int height) { assert(mgptr); mgptr->reshape(width, height); }
static void special_keyevent_wrapper(int key, int x, int y) { assert(mgptr); mgptr->special_keyevent(key, x, y); }
static void keyevent_wrapper(unsigned char key, int x, int y) { assert(mgptr); mgptr->keyevent(key, x, y); }
static void mouseupdown_wrapper(int button, int state, int x, int y) { assert(mgptr); mgptr->mouseupdown(button, state, x, y); }
static void mousemotion_wrapper(int x, int y) { assert(mgptr); mgptr->mousemotion(x, y); }

void MapGraphics::show(const char *title, int argc, char *argv[])
{
    assert(md);
    assert(mgui);
    assert(mgptr == NULL); // can't create MapGraphics twice
    mgptr = this;
    
    show_rtree = 0;
    query_description = L"No Query";
    clear_select();
    window_width = initial_window_height * md->map_ratio;
    window_height = initial_window_height;
    reset_display_range();
    
    /* glut things */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(window_width, window_height);
    glutCreateWindow(title);
    
    glutSpecialFunc(special_keyevent_wrapper);
    glutKeyboardFunc(keyevent_wrapper);
    glutMouseFunc(mouseupdown_wrapper);
    glutMotionFunc(mousemotion_wrapper);
    glutDisplayFunc(redraw_wrapper);
    glutReshapeFunc(reshape_wrapper);
    //glutIdleFunc(redraw_wrapper);
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glutMainLoop(); // never return
    assert(0);
}

