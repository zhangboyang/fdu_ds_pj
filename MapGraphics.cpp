#include <cmath>
#include <cstdio>
#include <GL/glut.h>
#include "common.h"
#include "MapData.h"
#include "MapGraphics.h"

using namespace std;

void MapGraphics::target(MapData *md) { MapGraphics::md = md; }

void MapGraphics::set_glimit(double gminx, double gmaxx, double gminy, double gmaxy)
{
    MapGraphics::gminx = gminx; MapGraphics::gmaxx = gmaxx; // set limit for MapGraphics
    MapGraphics::gminy = gminy; MapGraphics::gmaxy = gmaxy;
}

void MapGraphics::move_glimit(int x, int y)
{
    if (x != 0) {
        double diffx = (gmaxx - gminx) * MOVESTEP * x;
        gmaxx += diffx;
        gminx += diffx;
    }
    if (y != 0) {
        double diffy = (gmaxy - gminy) * MOVESTEP * y;
        gmaxy += diffy;
        gminy += diffy;
    }
}

void MapGraphics::zoom_glimit(int f)
{
    double diffx = (gmaxx - gminx) * (1 - pow(ZOOMSTEP, f));
    gmaxx -= diffx;
    gminx += diffx;
    
    double diffy = (gmaxy - gminy) * (1 - pow(ZOOMSTEP, f));
    gmaxy -= diffy;
    gminy += diffy;
}

void MapGraphics::get_gcoord_by_node(MapNode *node, double *gx, double *gy) // transform for MapGraphics
{
    assert(gmaxx - gminx > 0); assert(gmaxy - gminy > 0);
    *gx = (node->y - gminy) * md->gratio / (gmaxy - gminy);
    *gy = (node->x - gminx) / (gmaxx - gminx);
}

void MapGraphics::map_operation(MapGraphicsOperation op)
{
    printf("op=%d\n", op);
    if (op == UP) move_glimit(1, 0);
    else if (op == DOWN) move_glimit(-1, 0);
    else if (op == LEFT) move_glimit(0, -1);
    else if (op == RIGHT) move_glimit(0, 1);
    else if (op == ZOOMOUT) zoom_glimit(-1);
    else if (op == ZOOMIN) zoom_glimit(1);
    else assert(0);
}

static MapGraphics *mgptr = NULL;

void MapGraphics::redraw()
{
    //fflush(stdout);
    glClear(GL_COLOR_BUFFER_BIT);
    
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
            double gx, gy;
            //printf("%p\n", *it);
            get_gcoord_by_node(*nit, &gx, &gy);
            //printf("%f %f\n", gx, gy);
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

void redraw_wrapper() { assert(mgptr); TIMING("redraw", { mgptr->redraw(); }) }
void special_keyevent_wrapper(int key, int x, int y) { assert(mgptr); mgptr->special_keyevent(key, x, y); }

void MapGraphics::init_glut()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, md->gratio, 0.0, 1.0);
}

void MapGraphics::show(const char *title, int argc, char *argv[])
{
    assert(md);
    set_glimit(md->minx, md->maxx, md->miny, md->maxy); // FIXME: full map
    assert(mgptr == NULL);
    mgptr = this;
    
    /* glut things */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(md->mratio / md->gratio * WINDOWHEIGHT, WINDOWHEIGHT);
    glutCreateWindow(title);
    init_glut();
    glutSpecialFunc(special_keyevent_wrapper);
    glutDisplayFunc(redraw_wrapper);
    glutMainLoop();
    
    mgptr = NULL;
}

