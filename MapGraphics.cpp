#include <cstdio>
#include <GL/glut.h>
#include "common.h"
#include "MapData.h"
#include "MapGraphics.h"

using namespace std;

void MapGraphics::target(MapData *md) { MapGraphics::md = md; }

void MapGraphics::init_glut()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, md->gratio, 0.0, 1.0);
}

void MapGraphics::set_glimit(double gminx, double gmaxx, double gminy, double gmaxy)
{
    MapGraphics::gminx = gminx; MapGraphics::gmaxx = gmaxx; // set limit for MapGraphics
    MapGraphics::gminy = gminy; MapGraphics::gmaxy = gmaxy;
}

void MapGraphics::get_gcoord_by_node(MapNode *node, double *gx, double *gy) // transform for MapGraphics
{
    assert(gmaxx - gminx > 0); assert(gmaxy - gminy > 0);
    *gx = (node->y - gminy) * md->gratio / (gmaxy - gminy);
    *gy = (node->x - gminx) / (gmaxx - gminx);
}

static MapGraphics *mgptr;

void MapGraphics::redraw()
{
    //fflush(stdout);
    set_glimit(md->minx, md->maxx, md->miny, md->maxy); // FIXME: full map
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

void redraw_wrapper() { assert(mgptr); TIMING("redraw", { mgptr->redraw(); }) }

void MapGraphics::show(const char *title, int argc, char *argv[])
{
    assert(md);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(md->mratio / md->gratio * WINDOWHEIGHT, WINDOWHEIGHT);
    glutCreateWindow(title);
    init_glut();
    mgptr = this;
    glutDisplayFunc(redraw_wrapper);
    glutMainLoop();
    mgptr = NULL;
}

