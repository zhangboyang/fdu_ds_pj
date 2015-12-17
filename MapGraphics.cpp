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
#include "MapOperation.h"

#include <algorithm>
using namespace std;

void MapGraphics::target(MapData *md) { MapGraphics::md = md; }
void MapGraphics::target_gui(MapGUI *mgui) { MapGraphics::mgui = mgui; }
void MapGraphics::target_operation(MapOperation *mo)
{
    assert(md);
    assert(mgui);
    MapGraphics::mo = mo;
    mo->mg = this;
    mo->md = md;
    mo->mgui = mgui;
}

void MapGraphics::map_operation(MapOperation::MapOperationCode op)
{
    assert(mo);
    mo->operation(op);
    glutPostRedisplay();
}

double MapGraphics::get_display_resolution()
{
    return (md->maxy - md->miny) * pow(zoom_step, zoom_level);
}

void MapGraphics::update_current_display_level()
{
    clvl = md->ml.select_level(get_display_resolution());
}

/*void MapGraphics::trans_gcoord(double x, double y, double *gx, double *gy)
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
}*/


void MapGraphics::set_display_range(double dminx, double dmaxx, double dminy, double dmaxy)
{
    assert(dminx < dmaxx); assert(dminy < dmaxy);
    MapGraphics::dminx = dminx;
    MapGraphics::dminy = dminy;  // set limit for MapGraphics
    MapGraphics::dmaxx = dmaxx;
    MapGraphics::dmaxy = dmaxy;
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
        mo->snode = node;
        move_display_to_point(node->x, node->y);
    }
}

void MapGraphics::center_way(MapWay *way)
{
    if (way) {
        mo->sway = way;
        MapRect rect = way->get_rect();
        double x, y;
        x = (rect.left + rect.right) / 2;
        y = (rect.bottom + rect.top) / 2;
        move_display_to_point(x, y);
        zoom_display_by_size(rect.right - rect.left, rect.top - rect.bottom);
    }
}

void MapGraphics::highlight_point(MapNode *node, float color[], float thick)
{
    double x = node->x, y = node->y;
    double diff = selected_point_rect_size / 2.0 * (dmaxx - dminx) / window_width;
    glColor3f(color[0], color[1], color[2]);
    glLineWidth(thick);
    glPointSize(thick);
    glBegin(GL_LINE_LOOP);
    glVertex2d(x - diff, y - diff);
    glVertex2d(x - diff, y + diff);
    glVertex2d(x + diff, y + diff);
    glVertex2d(x + diff, y - diff);
    glEnd();
    glBegin(GL_POINTS);
    glVertex2d(x, y);
    glEnd();
}

void MapGraphics::draw_way(MapWay *way)
{
    glBegin(GL_LINE_STRIP);
    for (vector<MapNode *>::iterator nit = way->nl[clvl].begin(); nit != way->nl[clvl].end(); nit++) {
        MapNode *node = *nit;
        glVertex2d(node->x, node->y);
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
    
    update_current_display_level();
    printf("current display level: %d\n", clvl);
    
    dll.clear();
    md->lrt[clvl].find(dll, MapRect(dminx, dmaxx, dminy, dmaxy));
    printf("r-tree result lines: %lld\n", (LL) dll.size());

    // draw r-tree rectangles

    if (show_rtree) {
        vector<MapRect> rtree_rects;
        md->lrt[clvl].get_all_tree_rect(rtree_rects);
        printf("rect count = %lld\n", (LL) rtree_rects.size());
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.0f);
        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        for (vector<MapRect>::iterator rit = rtree_rects.begin(); rit != rtree_rects.end(); rit++) {
            MapRect &rect = *rit;
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
    for (int num = 0; num < MapOperation::MAX_KBDNUM; num++) {
        MapWay *way = mo->nway[num];
        if (way) {
            if (way == mo->sway) sway_flag = true;
            glColor3f(ncolor[num][0], ncolor[num][1], ncolor[num][2]);
            glLineWidth(selected_way_thick);
            draw_way(way);
        }
    }
    if (mo->sway && !sway_flag) {
        glColor3f(scolor[0], scolor[1], scolor[2]);
        glLineWidth(selected_way_thick);
        draw_way(mo->sway);
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
    for (int num = 0; num < MapOperation::MAX_KBDNUM; num++) {
        MapNode *node = mo->nnode[num];
        if (node) {
            highlight_point(node, ncolor[num], selected_point_rect_thick);
            if (node == mo->snode) snode_flag = true;
        }
    }
    if (mo->snode && !snode_flag)
        highlight_point(mo->snode, scolor, selected_point_rect_thick);
    
    glFlush();
}

void MapGraphics::special_keyevent(int key, int x, int y)
{
    MapOperation::MapOperationCode op;
    switch (key) {
        case GLUT_KEY_F1: op = MapOperation::RESET_VIEW; break;
        case GLUT_KEY_F2: op = MapOperation::TOGGLE_RTREE; break;
        case GLUT_KEY_F3: op = MapOperation::QUERY_NAME; break;
        case GLUT_KEY_F4: op = MapOperation::CLEAR_SELECT; break;
        case GLUT_KEY_F8: op = MapOperation::SHOW_QUERY_RESULT; break;
        case GLUT_KEY_F9: op = MapOperation::CENTER_SEL_POINT; break;
        case GLUT_KEY_F10: op = MapOperation::CENTER_SEL_WAY; break;
        case GLUT_KEY_F11: op = MapOperation::SHOW_NODEINFO; break;
        case GLUT_KEY_F12: op = MapOperation::SHOW_WAYINFO; break;
        case GLUT_KEY_UP: op = MapOperation::MOVE_UP; break;
        case GLUT_KEY_DOWN: op = MapOperation::MOVE_DOWN; break;
        case GLUT_KEY_LEFT: op = MapOperation::MOVE_LEFT; break;
        case GLUT_KEY_RIGHT: op = MapOperation::MOVE_RIGHT; break;
        case GLUT_KEY_PAGE_UP: op = MapOperation::ZOOM_OUT; break;
        case GLUT_KEY_PAGE_DOWN: op = MapOperation::ZOOM_IN; break;
        default: printf("unknown key %d\n", key); return;
    }
    map_operation(op);
}

void MapGraphics::keyevent(unsigned char key, int x, int y)
{
    MapOperation::MapOperationCode op;
    char kbd_char = key;
    if (isupper(kbd_char)) kbd_char = kbd_char - 'A' + 'a';
    int mkey = glutGetModifiers();
    int shift = mkey & GLUT_ACTIVE_SHIFT;
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
        assert(kbd_num < MapOperation::MAX_KBDNUM);
        if (shift)
            op = MapOperation::NUMBER_POINT;
        else
            op = MapOperation::CENTER_NUM_POINT;
    } else if (strchr("qwertyuiop", kbd_char)) {
        const char *s = "qwertyuiop";
        kbd_num = strchr(s, kbd_char) - s;
        assert(kbd_num < MapOperation::MAX_KBDNUM);
        if (shift)
            op = MapOperation::NUMBER_WAY;
        else
            op = MapOperation::CENTER_NUM_WAY;
    } else switch (kbd_char) {
        case '-': case '_' : op = MapOperation::ZOOM_OUT; break;
        case '+': case '=' : op = MapOperation::ZOOM_IN; break;
        case 'k': op = MapOperation::MOVE_UP; break;
        case 'j': op = MapOperation::MOVE_DOWN; break;
        case 'h': op = MapOperation::MOVE_LEFT; break;
        case 'l': op = MapOperation::MOVE_RIGHT; break;
        case '\x1b': op = MapOperation::POP_DISPLAY; break; // ESC
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
    MapOperation::MapOperationCode op;
    if (use_last_op) {
        op = last_mouse_op;
    } else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        op = MapOperation::SELECT_WAY;
    } else if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        op = MapOperation::SELECT_POINT;
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



static MapGraphics *mgptr = NULL;
//void redraw_wrapper() { assert(mgptr); mgptr->redraw(); }
static void redraw_wrapper() { assert(mgptr); TIMING("redraw", { mgptr->redraw(); }) }
static void reshape_wrapper(int width, int height) { assert(mgptr); mgptr->reshape(width, height); }
static void special_keyevent_wrapper(int key, int x, int y) { assert(mgptr); mgptr->special_keyevent(key, x, y); }
static void keyevent_wrapper(unsigned char key, int x, int y) { assert(mgptr); mgptr->keyevent(key, x, y); }
static void mouseupdown_wrapper(int button, int state, int x, int y) { assert(mgptr); mgptr->mouseupdown(button, state, x, y); }
static void mousemotion_wrapper(int x, int y) { assert(mgptr); mgptr->mousemotion(x, y); }

void MapGraphics::show(const char *title, int argc, char *argv[])
{
    assert(mo);
    assert(md);
    assert(mgui);
    assert(mgptr == NULL); // can't create MapGraphics twice
    mgptr = this;
    
    show_rtree = 0;
    mo->query_description = L"No Query";
    mo->clear_select();
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

