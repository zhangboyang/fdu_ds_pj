#include <cmath>
#include <cstdio>
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
    printf("INIT!\n");
    mgui->prepare_inputbox();
    mgui->set_inputbox_title(L"i am title");
    mgui->set_inputbox_description(L"set_msgbox_description");
    mgui->set_inputbox_text(L"text!");
    std::wstring uinput = mgui->show_inputbox();
    
    printf("str=%s\n", ws2s(uinput).c_str());
    
    mgui->prepare_msgbox();
    mgui->set_msgbox_title(s2ws("哈哈哈！"));
    mgui->set_msgbox_description(L"set_msgbox_description");
    mgui->set_msgbox_append(L"text!");
    mgui->set_msgbox_append(uinput);
    mgui->show_msgbox();
    printf("FINISH!\n");
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

    double mdist = F_INF;
    MapPoint P(cx, cy);
    for (vector<MapWay *>::iterator wit = dwl.begin(); wit != dwl.end(); wit++) {
        MapWay *way = *wit;
        MapPoint A, B;
        for (vector<MapNode *>::iterator nit = way->nl.begin(); nit != way->nl.end(); nit++) {
            MapNode *node = *nit;
            B = node->get_point();
            if (nit != way->nl.begin()) {
                double dist = dts(P, A, B);
                if (dist < mdist) { mdist = dist; sway = way; }
            }
            A = B;
        }
    }
}


void MapGraphics::select_point()
{
    double cx, cy; // cursor, map coord
    rtrans_gcoord(mx, my, &cx, &cy);

    double mdist = F_INF;
    MapPoint P(cx, cy);
    for (vector<MapWay *>::iterator wit = dwl.begin(); wit != dwl.end(); wit++) {
        MapWay *way = *wit;
        MapPoint A, B;
        for (vector<MapNode *>::iterator nit = way->nl.begin(); nit != way->nl.end(); nit++) {
            MapNode *node = *nit;
            A = node->get_point();
            double dist = len(A - P);
            if (dist < mdist) { mdist = dist; snode = node; }
        }
    }
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

void MapGraphics::map_operation(MapGraphicsOperation op)
{
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
        default: assert(0); break;
    }
    glutPostRedisplay();
}

static MapGraphics *mgptr = NULL;

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
    
    for (vector<MapWay *>::iterator wit = dwl.begin(); wit != dwl.end(); wit++) {
        MapWay *way = *wit;
        float r, g, b, thickness;
        md->wt.query_rgbt(way->waytype, &r, &g, &b, &thickness);
        
        if (way == sway) {
            glColor3f(scolor[0], scolor[1], scolor[2]);
            glLineWidth(selected_way_thick);
        } else {
            glColor3f(r, g, b);
            glLineWidth(thickness);
        }
        glBegin(GL_LINE_STRIP);
        for (vector<MapNode *>::iterator nit = way->nl.begin(); nit != way->nl.end(); nit++) {
            MapNode *node = *nit;
            double gx, gy;
            trans_gcoord(node->x, node->y, &gx, &gy);
            glVertex2d(gx, gy);
        }
        glEnd();
    }
    
    if (snode) {
        double gx, gy;
        trans_gcoord(snode->x, snode->y, &gx, &gy);
        double diff = selected_point_rect_size / 2.0 * (dmaxx - dminx) / window_width;
        glColor3f(scolor[0], scolor[1], scolor[2]);
        glLineWidth(selected_point_rect_thick);
        glBegin(GL_LINE_LOOP);
        glVertex2d(gx - diff, gy - diff);
        glVertex2d(gx - diff, gy + diff);
        glVertex2d(gx + diff, gy + diff);
        glVertex2d(gx + diff, gy - diff);
        glEnd();
    }
    
    glFlush();
}

void MapGraphics::special_keyevent(int key, int x, int y)
{
    MapGraphicsOperation op;
    switch (key) {
        case GLUT_KEY_F1: op = RESET_VIEW; break;
        case GLUT_KEY_F2: op = TOGGLE_RTREE; break;
        case GLUT_KEY_F3: op = QUERY_NAME; break;
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
static void mouseupdown_wrapper(int button, int state, int x, int y) { assert(mgptr); mgptr->mouseupdown(button, state, x, y); }
static void mousemotion_wrapper(int x, int y) { assert(mgptr); mgptr->mousemotion(x, y); }

void MapGraphics::show(const char *title, int argc, char *argv[])
{
    assert(md);
    assert(mgui);
    assert(mgptr == NULL); // can't create MapGraphics twice
    mgptr = this;
    
    show_rtree = 0;
    window_width = initial_window_height * md->map_ratio;
    window_height = initial_window_height;
    reset_display_range();
    
    /* glut things */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(window_width, window_height);
    glutCreateWindow(title);
    
    glutSpecialFunc(special_keyevent_wrapper);
    glutMouseFunc(mouseupdown_wrapper);
    glutMotionFunc(mousemotion_wrapper);
    glutDisplayFunc(redraw_wrapper);
    glutReshapeFunc(reshape_wrapper);
    //glutIdleFunc(redraw_wrapper);
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glutMainLoop(); // never return
    assert(0);
}

