#include <cmath>
#include <cstdio>
#include <GL/glut.h>
#include "common.h"
#include "MapData.h"
#include "MapGraphics.h"
#include "MapRect.h"

#include <algorithm>
using namespace std;

void MapGraphics::target(MapData *md) { MapGraphics::md = md; }

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
}

void MapGraphics::move_display_range(int x, int y)
{
    if (x != 0) {
        double diffx = (dmaxx - dminx) * MOVESTEP * x;
        dmaxx += diffx;
        dminx += diffx;
    }
    if (y != 0) {
        double diffy = (dmaxy - dminy) * MOVESTEP * y;
        dmaxy += diffy;
        dminy += diffy;
    }
}

void MapGraphics::zoom_display_range(int f)
{
    double diffx = (dmaxx - dminx) * (1 - pow(ZOOMSTEP, f));
    dmaxx -= diffx;
    dminx += diffx;
    
    double diffy = (dmaxy - dminy) * (1 - pow(ZOOMSTEP, f));
    dmaxy -= diffy;
    dminy += diffy;
}

void MapGraphics::reset_display_range()
{
    set_display_range(md->minx, md->maxx, md->miny, md->maxy);
}

void MapGraphics::map_operation(MapGraphicsOperation op)
{
    if (op == UP) move_display_range(0, 1);
    else if (op == DOWN) move_display_range(0, -1);
    else if (op == LEFT) move_display_range(-1, 0);
    else if (op == RIGHT) move_display_range(1, 0);
    else if (op == ZOOMOUT) zoom_display_range(-1);
    else if (op == ZOOMIN) zoom_display_range(1);
    else if (op == RESETVIEW) reset_display_range();
    else assert(0);
}

static MapGraphics *mgptr = NULL;

void MapGraphics::redraw()
{
    //fflush(stdout);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(dminx, dmaxx, dminy, dmaxy);
    printd("range: %f %f %f %f\n", dminx, dmaxx, dminy, dmaxy);
    
    double mminx, mmaxx, mminy, mmaxy;
    rtrans_gcoord(dminx, dminy, &mminx, &mminy);
    rtrans_gcoord(dmaxx, dmaxy, &mmaxx, &mmaxy);
    
    vector<MapLine *> result;
    md->lrt.find(result, MapRect(mminx, mmaxx, mminy, mmaxy));
    
    printf("r-tree result count: %lld\n", (LL) result.size());
    
    vector<MapWay *> dwl;
    for (vector<MapLine *>::iterator lit = result.begin(); lit != result.end(); lit++)
        dwl.push_back((*lit)->way);
    sort(dwl.begin(), dwl.end());
    dwl.resize(unique(dwl.begin(), dwl.end()) - dwl.begin());
    
    for (vector<MapWay *>::iterator wit = dwl.begin(); wit != dwl.end(); wit++) {
        //if (wit - md->wl.begin() >= x) { x++; break; }
        //printf("drawing %lld\n", (*wit)->id);
        glBegin(GL_LINE_STRIP);
        MapWay *way = *wit;
        for (vector<MapNode *>::iterator nit = way->nl.begin(); nit != way->nl.end(); nit++) {
            MapNode *node = *nit;
            double gx, gy;
            trans_gcoord(node->x, node->y, &gx, &gy);
            glVertex2d(gx, gy);
        }
        glEnd();
    }
    
    
    glFlush();
}

void MapGraphics::special_keyevent(int key, int x, int y)
{
    MapGraphicsOperation op;
    switch (key) {
        case GLUT_KEY_F1: op = RESETVIEW; break;
        case GLUT_KEY_UP: op = UP; break;
        case GLUT_KEY_DOWN: op = DOWN; break;
        case GLUT_KEY_LEFT: op = LEFT; break;
        case GLUT_KEY_RIGHT: op = RIGHT; break;
        case GLUT_KEY_PAGE_UP: op = ZOOMOUT; break;
        case GLUT_KEY_PAGE_DOWN: op = ZOOMIN; break;
        default: printf("unknown key %d\n", key); return;
    }
    map_operation(op);
    glutPostRedisplay();
}

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


void redraw_wrapper() { assert(mgptr); TIMING("redraw", { mgptr->redraw(); }) }
void reshape_wrapper(int width, int height) { assert(mgptr); mgptr->reshape(width, height); }
void special_keyevent_wrapper(int key, int x, int y) { assert(mgptr); mgptr->special_keyevent(key, x, y); }


void MapGraphics::show(const char *title, int argc, char *argv[])
{
    assert(md);
    reset_display_range(); // FIXME: full map
    assert(mgptr == NULL); // can't create MapGraphics twice
    mgptr = this;
    
    /* glut things */
    window_width = INITIAL_WINDOW_HEIGHT * md->map_ratio;
    window_height = INITIAL_WINDOW_HEIGHT;
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(window_width, window_height);
    glutCreateWindow(title);
    
    glutSpecialFunc(special_keyevent_wrapper);
    glutDisplayFunc(redraw_wrapper);
    glutReshapeFunc(reshape_wrapper);
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glutMainLoop(); // never return
    assert(0);
}

