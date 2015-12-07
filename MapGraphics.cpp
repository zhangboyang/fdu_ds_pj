#include <cstdio>
#include <GL/glut.h>
#include "common.h"
#include "MapData.h"
#include "MapGraphics.h"

using namespace std;

void MapGraphics::target(MapData *md) { MapGraphics::md = md; }

void init_glut()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0.0, MapGraphics::MAPWIDTH, 0.0, MapGraphics::MAPHEIGHT);
}

static MapGraphics *mgptr;

void MapGraphics::redraw()
{
    //fflush(stdout);
    MapNode::set_glimit(md->minx, md->maxx, md->miny, md->maxy); // FIXME: full map
    glClear(GL_COLOR_BUFFER_BIT);
    
    /*for (vector<MapNode *>::iterator it = md->nl.begin(); it != md->nl.end(); it++) {
        double gx, gy;
        //printf("%p\n", *it);
        (*it)->get_gcoord(&gx, &gy);
        printf("%f %f\n", gx, gy);
        glVertex2d(gx, gy);
    }*/
    
    for (vector<MapWay *>::iterator wit = md->wl.begin(); wit != md->wl.end(); wit++) {
        glBegin(GL_LINE_LOOP);
        MapWay *way = *wit;
        for (vector<MapNode *>::iterator nit = way->nl.begin(); nit != way->nl.end(); nit++) {
            double gx, gy;
            //printf("%p\n", *it);
            (*nit)->get_gcoord(&gx, &gy);
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

