#include <cmath>
#include <cstdio>
#include <GL/glut.h>
#include "common.h"
#include "MapData.h"
#include "MapGraphics.h"

using namespace std;

void MapGraphics::target(MapData *md) { MapGraphics::md = md; }

void MapGraphics::set_display_range(double dminx, double dmaxx, double dminy, double dmaxy)
{
    MapGraphics::dminx = dminx; MapGraphics::dmaxx = dmaxx; // set limit for MapGraphics
    MapGraphics::dminy = dminy; MapGraphics::dmaxy = dmaxy;
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

void MapGraphics::map_operation(MapGraphicsOperation op)
{
    if (op == UP) move_display_range(0, 1);
    else if (op == DOWN) move_display_range(0, -1);
    else if (op == LEFT) move_display_range(-1, 0);
    else if (op == RIGHT) move_display_range(1, 0);
    else if (op == ZOOMOUT) zoom_display_range(-1);
    else if (op == ZOOMIN) zoom_display_range(1);
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
    printf("%f %f %f %f\n", dminx, dmaxx, dminy, dmaxy);
    
    /*for (vector<MapNode *>::iterator it = md->nl.begin(); it != md->nl.end(); it++) {
        double gx, gy;
        //printf("%p\n", *it);
        (*it)->get_gcoord(&gx, &gy);
        printf("%f %f\n", gx, gy);
        glVertex2d(gx, gy);
    }*/
    
    //static int x = 0;
    for (vector<MapWay *>::iterator wit = md->wl.begin(); wit != md->wl.end(); wit++) {
        //if (wit - md->wl.begin() >= x) { x++; break; }
        //printf("drawing %lld\n", (*wit)->id);
        glBegin(GL_LINE_STRIP);
        MapWay *way = *wit;
        for (vector<MapNode *>::iterator nit = way->nl.begin(); nit != way->nl.end(); nit++) {
            MapNode *node = *nit;
            glVertex2d(node->x, node->y);
        }
        glEnd();
    }
    
    
    glFlush();
}

void MapGraphics::special_keyevent(int key, int x, int y)
{
    MapGraphicsOperation op;
    switch (key) {
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
    set_display_range(md->minx, md->maxx, md->miny, md->maxy); // FIXME: full map
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

