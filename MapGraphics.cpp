#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include "common.h"
#include "MapData.h"
#include "MapGraphics.h"
#include "MapRect.h"
#include "MapGUI.h"
#include "wstr.h"
#include "MapVector.h"
#include "MapOperation.h"
#include "printf2str.h"
#include "myclock.h"

#include <algorithm>
using namespace std;

void MapGraphics::target(MapData *md) { MapGraphics::md = md; }
void MapGraphics::target_gui(MapGUI *mgui) { MapGraphics::mgui = mgui; }
void MapGraphics::target_shortestpath(MapShortestPath *msp) { MapGraphics::msp = msp; }
void MapGraphics::target_operation(MapOperation *mo) // must be called after other target()
{
    assert(md);
    assert(mgui);
    assert(msp);
    MapGraphics::mo = mo;
    mo->mg = this;
    mo->md = md;
    mo->mgui = mgui;
    mo->msp = msp;
    msp->md = md;
}

void MapGraphics::map_operation(MapOperation::MapOperationCode op)
{
    assert(mo);
    double st_clock, ed_clock;
    st_clock = myclock();
    mo->operation(op);
    ed_clock = myclock();
    last_operation_time = ed_clock - st_clock;
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

void MapGraphics::trans_gcoord(double x, double y, double *gx, double *gy)
{
    *gx = x - md->minx;
    *gy = y - md->miny;
}

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

void MapGraphics::get_display_range(double *minx, double *maxx, double *miny, double *maxy)
{
    *minx = MapGraphics::dminx;
    *miny = MapGraphics::dminy;
    *maxx = MapGraphics::dmaxx;
    *maxy = MapGraphics::dmaxy;
}

void MapGraphics::push_display_range()
{
    double centerx = dminx + (dmaxx - dminx) / 2;
    double centery = dminy + (dmaxy - dminy) / 2;
    if (display_stack.empty() ||
            (!fequ(display_stack.back().first.first, centerx) ||
             !fequ(display_stack.back().first.second, centery) ||
             display_stack.back().second != zoom_level)) {
        display_stack.push_back(make_pair(make_pair(centerx, centery), zoom_level));
    }
}

void MapGraphics::pop_display_range()
{
    if (!display_stack.empty()) {
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

void MapGraphics::zoom_display_range(int f, bool by_mouse)
{
    double fx = 0.5, fy = 0.5;
    if (by_mouse) {
        fx = (mx - dminx) / (dmaxx - dminx);
        fy = (my - dminy) / (dmaxy - dminy);
    } 
    
    double diffx = (dmaxx - dminx) * (1 - pow(zoom_step, f));
    double diffy = (dmaxy - dminy) * (1 - pow(zoom_step, f));
    double yres = fabs(diffy) / window_height;
    
    printd("diffx = %e, diffy = %e, yres = %e\n", diffx, diffy, yres);
    if (yres < zoom_low_limit || yres > zoom_high_limit) {
        msg.append("ERROR:\n  Zoom level out of range.");
        return;
    }
    
    dmaxx -= diffx * (1 - fx);
    dminx += diffx * fx;
    dmaxy -= diffy * (1 - fy);
    dminy += diffy * fy;
    
    zoom_level += f;
}


/*void MapGraphics::zoom_display_by_size(double sizex, double sizey)
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
}*/

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
        //zoom_display_by_size(rect.right - rect.left, rect.top - rect.bottom);
    }
}

void MapGraphics::draw_vertex(double x, double y)
{
    double gx, gy;
    trans_gcoord(x, y, &gx, &gy);
    glVertex2d(gx, gy);
    vertex_count++;
}

void MapGraphics::highlight_point(MapNode *node, double size, float color[], float thick)
{
    double x = node->x, y = node->y;
    double diff = size / 2.0 * (dmaxx - dminx) / window_width;
    glColor3f(color[0], color[1], color[2]);
    glLineWidth(thick);
    glPointSize(thick);
    glBegin(GL_LINE_LOOP);
    draw_vertex(x - diff, y - diff);
    draw_vertex(x - diff, y + diff);
    draw_vertex(x + diff, y + diff);
    draw_vertex(x + diff, y - diff);
    glEnd();
    glBegin(GL_POINTS);
    draw_vertex(x, y);
    glEnd();
}

void MapGraphics::draw_way(MapWay *way, bool force_level)
{
    int lvl = force_level ? 0 : clvl;
    glBegin(GL_LINE_STRIP);
    for (vector<MapNode *>::iterator nit = way->nl[lvl].begin(); nit != way->nl[lvl].end(); nit++) {
        MapNode *node = *nit;
        draw_vertex(node->x, node->y);
    }
    glEnd();
}

void MapGraphics::print_string(const char *str)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 1, 0, 1);
    glColor3d(1.0, 1.0, 0.0);
    const double char_height = 14, char_width = 8, padding = 2;
    int cnt = 0;
    int maxrow = 1, maxcol = 0;
    const char *ptr;
    for (ptr = str; *ptr; ptr++)
        if (*ptr == '\n')
            cnt = 0, maxrow++;
        else if (isprint(*ptr))
            maxcol = max(maxcol, ++cnt);
    while (ptr > str && *--ptr == '\n') maxrow--;
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_QUADS);
    double x1 = 0, x2 = (maxcol * char_width + padding) / window_width;
    double y1 = 1, y2 = 1 - (maxrow * char_height + padding) / window_height;
    glVertex2d(x1, y1);
    glVertex2d(x1, y2);
    glVertex2d(x2, y2);
    glVertex2d(x2, y1);
    glEnd();
    
    glColor3d(1.0, 1.0, 1.0);
    glRasterPos2d(0, 1 - char_height / window_height);    
    glutBitmapString(GLUT_BITMAP_8_BY_13, (const unsigned char *) str);
}

void MapGraphics::reload_vertex()
{
    int vl_size = 0, il_size = 0;
    std::map<float, std::pair<std::vector<vnode>, std::vector<unsigned> > > &tvil = vct_tvil[clvl];
    std::vector<vnode> &vl = vct_vl[clvl];
    std::vector<unsigned> &il = vct_il[clvl];
    std::vector<std::pair<float, std::pair<int, int> > > &tl = vct_tl[clvl];
    
    // put vertex and indice to tvil
    for (vector<MapWay *>::iterator wit = dwl.begin(); wit != dwl.end(); wit++) {
        MapWay *way = *wit;
        int last_vn_id = -1;
        vnode vn;
        float thickness;
        md->wt.query_rgbt(way->waytype, &vn.r, &vn.g, &vn.b, &thickness);
        pair<vector<vnode>, vector<unsigned> > &p = tvil[thickness];
        for (vector<MapNode *>::iterator nit = way->nl[clvl].begin(); nit != way->nl[clvl].end(); nit++) {
            MapNode *node = *nit;
            double gx, gy;
            trans_gcoord(node->x, node->y, &gx, &gy);
            vn.x = gx;
            vn.y = gy;
            int cur_vn_id = p.first.size();
            p.first.push_back(vn); vl_size++;
            if (last_vn_id >= 0) { // need to draw line
                p.second.push_back(last_vn_id);
                p.second.push_back(cur_vn_id);
                il_size++;
            }
            last_vn_id = cur_vn_id;
        }
    }
    
    // construct vl, il, tl
    vl.clear(); vl.reserve(vl_size);
    il.clear(); il.reserve(il_size);
    tl.clear();
    for (map<float, pair<vector<vnode>, vector<unsigned> > >::iterator tvilit = tvil.begin(); tvilit != tvil.end(); tvilit++) {
        vector<vnode> &tvl = tvilit->second.first;
        vector<unsigned> &til = tvilit->second.second;
        tl.push_back(make_pair(tvilit->first, make_pair((int) il.size(), (int) til.size())));
        for (vector<unsigned>::iterator tilit = til.begin(); tilit != til.end(); tilit++)
            il.push_back(*tilit + vl.size());
        vl.insert(vl.end(), tvl.begin(), tvl.end());
    }
    
    // clean up tvil
    for (map<float, pair<vector<vnode>, vector<unsigned> > >::iterator tvilit = tvil.begin(); tvilit != tvil.end(); tvilit++) {
        vector<vnode> &tvl = tvilit->second.first;
        vector<unsigned> &til = tvilit->second.second;
        tvl.clear();
        til.clear();
    }
}

void MapGraphics::reload_dll(double minx, double maxx, double miny, double maxy)
{
    double st_clock = myclock();
    dll.clear();
    md->lrt[clvl].find(dll, MapRect(minx, maxx, miny, maxy));
    dll_reload_time = myclock() - st_clock;
    printd("r-tree: %f ms\n", dll_reload_time);
}

void MapGraphics::reload_dwl()
{
    double st_clock = myclock();
    dwl.clear();
    for (vector<MapLine *>::iterator lit = dll.begin(); lit != dll.end(); lit++)
        dwl.push_back((*lit)->way);
    sort(dwl.begin(), dwl.end());
    dwl.resize(unique(dwl.begin(), dwl.end()) - dwl.begin());
    dwl_reload_time = myclock() - st_clock;
    printd("reload: %f ms\n", dwl_reload_time);
}

void MapGraphics::load_current_level_buffer()
{
    std::vector<vnode> &vl = vct_vl[clvl];
    std::vector<unsigned> &il = vct_il[clvl];
    
    // vertex
    glBindBuffer(GL_ARRAY_BUFFER, vbuffer[clvl]);
    glVertexPointer(2, GL_FLOAT, sizeof(vnode), 0);
    glColorPointer(3, GL_FLOAT, sizeof(vnode), (const void *) offsetof(vnode, r));
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vnode) * vl.size(), &vl[0], GL_STATIC_DRAW);
    
    // indice
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer[clvl]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * il.size(), &il[0], GL_STATIC_DRAW);
}

void MapGraphics::redraw()
{
    double st_clock, ed_clock;
    st_clock = myclock();
    vertex_count = 0;
    
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (!is_dragging_map) {
        double gminx, gmaxx, gminy, gmaxy;
        trans_gcoord(dminx, dminy, &gminx, &gminy);
        trans_gcoord(dmaxx, dmaxy, &gmaxx, &gmaxy);
        printd("range: %f %f %f %f\n", gminx, gmaxx, gminy, gmaxy);
        gluOrtho2D(gminx, gmaxx, gminy, gmaxy);
    } else {
        double diff_mx = last_mx - mx;
        double diff_my = last_my - my;
        double tminx, tmaxx, tminy, tmaxy;
        tminx = dminx + diff_mx; tmaxx = dmaxx + diff_mx;
        tminy = dminy + diff_my; tmaxy = dmaxy + diff_my;
        double gminx, gmaxx, gminy, gmaxy;
        trans_gcoord(tminx, tminy, &gminx, &gminy);
        trans_gcoord(tmaxx, tmaxy, &gmaxx, &gmaxy);
        gluOrtho2D(gminx, gmaxx, gminy, gmaxy);
    }
    
    glShadeModel(GL_FLAT);
    
    bool lvl_changed = false;
    update_current_display_level();
    if (clvl != last_clvl) lvl_changed = true;
    last_clvl = clvl;
        
    dwl_reload_time = dll_reload_time = -1;
    // check if we need to reload line data or vertex data
    if (rtminx != dminx || rtmaxx != dmaxx ||
            rtminy != dminy || rtmaxy != dmaxy || dll.empty()) { // need re-query r-tree
        reload_dll(dminx, dmaxx, dminy, dmaxy);
        if (use_rtree_for_drawing) { // if all data have been loaded to gpu, no reload needed
            reload_dwl();
            reload_vertex();
            load_current_level_buffer();
        }
        rtminx = dminx; rtmaxx = dmaxx;
        rtminy = dminy; rtmaxy = dmaxy;
    }

    // load vertex buffer by current level, only needed when no r-tree
    if (!use_rtree_for_drawing && lvl_changed)
        load_current_level_buffer();
    
    // draw r-tree rectangles
    if (show_rtree) {
        vector<MapRect> rtree_rects;
        md->lrt[clvl].get_all_tree_rect(rtree_rects);
        printd("rect count = %lld\n", (LL) rtree_rects.size());
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.0f);
        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        for (vector<MapRect>::iterator rit = rtree_rects.begin(); rit != rtree_rects.end(); rit++) {
            MapRect &rect = *rit;
            draw_vertex(rect.left, rect.bottom);
            draw_vertex(rect.right, rect.bottom);
            draw_vertex(rect.right, rect.top);
            draw_vertex(rect.left, rect.top);
        }
        glEnd();
    }
    
    
    bool sway_flag = false;
    // draw numbered ways
    for (int num = 0; num < MapOperation::MAX_KBDNUM; num++) {
        MapWay *way = mo->nway[num];
        if (way) {
            if (way == mo->sway) sway_flag = true;
            glColor3f(ncolor[num][0], ncolor[num][1], ncolor[num][2]);
            glLineWidth(selected_way_thick);
            draw_way(way, true);
        }
    }
    // draw selected way
    if (mo->sway && !sway_flag) {
        glColor3f(scolor[0], scolor[1], scolor[2]);
        glLineWidth(selected_way_thick);
        draw_way(mo->sway, true);
    }
    // draw normal ways
    // speed is very important, use glDrawElements() instead of glBegin()...glEnd()
    std::vector<std::pair<float, std::pair<int, int> > > &tl = vct_tl[clvl];
    for (vector<pair<float, pair<int, int> > >::iterator tlit = tl.begin(); tlit != tl.end(); tlit++) {
        glLineWidth(tlit->first);
        glDrawElements(GL_LINES, tlit->second.second, GL_UNSIGNED_INT, (const void *) (tlit->second.first * sizeof(unsigned)));
    }
    // draw result ways
    for (vector<MapWay *>::iterator wit = mo->wresult.begin(); wit != mo->wresult.end(); wit++) {
        MapWay *way = *wit;
        bool wflag = false;
        for (int num = 0; num < MapOperation::MAX_KBDNUM; num++)
            if (mo->nway[num] == way) {
                wflag = true;
                break;
            }
        if (!wflag) {
            glColor3f(wrcolor[0], wrcolor[1], wrcolor[2]);
            glLineWidth(wrthick);
            draw_way(way, true);
        }
    }
    // draw shortest path
    if (!mo->sp_result.empty()) {
        glLineWidth(sp_path_thick);
        glColor3f(sp_path_color[0], sp_path_color[1], sp_path_color[2]);
        glBegin(GL_LINES);
        for (vector<MapLine *>::iterator lit = mo->sp_result.begin(); lit != mo->sp_result.end(); lit++) {
            MapLine *line = *lit;
            MapNode *from = line->p1, *to = line->p2;
            draw_vertex(from->x, from->y);
            draw_vertex(to->x, to->y);
        }
        glEnd();
    }
    // draw shortest-path-tree
    if (show_shortestpath_node && !mo->sp_result.empty()) {
        glColor3f(0.0f, 1.0f, 0.0f);
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        for (vector<MapNode *>::iterator nit = md->nl.begin(); nit != md->nl.end(); nit++) {
            MapLine *line = (*nit)->from;
            if (line) {
                draw_vertex(line->p1->x, line->p1->y);
                draw_vertex(line->p2->x, line->p2->y);
            }
        }
        glEnd();
    }
    
    
    bool snode_flag = false;
    // draw numbered nodes
    for (int num = 0; num < MapOperation::MAX_KBDNUM; num++) {
        MapNode *node = mo->nnode[num];
        if (node) {
            highlight_point(node, selected_point_rect_size, ncolor[num], selected_point_rect_thick);
            if (node == mo->snode) snode_flag = true;
        }
    }
    // draw selected node
    if (mo->snode && !snode_flag)
        highlight_point(mo->snode, selected_point_rect_size, scolor, selected_point_rect_thick);
    for (vector<MapNode *>::iterator nit = mo->nresult.begin(); nit != mo->nresult.end(); nit++) {
        MapNode *node = *nit;
        bool nflag = false;
        for (int num = 0; num < MapOperation::MAX_KBDNUM; num++)
            if (mo->nnode[num] == node) {
                nflag = true;
                break;
            }
        if (!nflag) highlight_point(node, nrsize, nrcolor, nrthick);
    }
    // draw poly vertex
    if (mo->pvl.size() > 1) {
        glColor3f(pcolor[0], pcolor[1], pcolor[2]);
        glLineWidth(pthickness);
        glBegin(GL_LINE_LOOP);
        for (vector<MapPoint>::iterator it = mo->pvl.begin(); it != mo->pvl.end(); it++) {
            draw_vertex(it->x, it->y);
        }
        glEnd();
    }
    // draw shortest path src and dest
    if (mo->sp_start) highlight_point(mo->sp_start, sp_vertex_rect_size, sp_src_color, sp_vertex_rect_thick);
    if (mo->sp_end) highlight_point(mo->sp_end, sp_vertex_rect_size, sp_dest_color, sp_vertex_rect_thick);
    
    
    // draw messages
    string redraw_str = printf2str(
        "Level: %d\nWay: %lld\nLine: %lld\nVertex: %d+%d\nMouse: %s\nSP Algo: %s\nOperation: %.2f ms\n",
            clvl, (LL) dwl.size(), (LL) dll.size(), vertex_count, (int) vct_vl[clvl].size(), 
            select_flag ? "Select" : "Drag", msp->get_algo_name(mo->sp_algo),last_operation_time);
    if (dll_reload_time >= 0) redraw_str += printf2str("R-tree: %.2f ms\n", dll_reload_time); // vertex reloaded
    if (dwl_reload_time >= 0) redraw_str += printf2str("Unique: %.2f ms\n", dwl_reload_time); // vertex reloaded

    
    if (show_message) {
        // construct strings
        string snode_str;
        if (mo->snode) {
            snode_str = "== Selected Node ==\n";
            MapNode *snode = mo->snode;
            snode_str += printf2str(
                "ID: %lld\nLat: %f\nLon: %f\nSP: %s\n",
                snode->id, snode->lat, snode->lon, snode->on_shortest_path ? "YES" : "NO");
        }
        string sway_str;
        if (mo->sway) {
            sway_str = "== Selected Way ==\n";
            MapWay *sway = mo->sway;
            sway_str += printf2str("ID: %lld\n", sway->id);
            double way_length = sway->calc_length() * MapData::dist_factor;
            sway_str += "Length: " + md->get_length_string(way_length) + '\n';
            if (sway->is_closed()) {
                double way_area = sway->calc_area() * sq(MapData::dist_factor);
                sway_str += "Ares: " + md->get_area_string(way_area) + '\n';
            }
            sway_str += printf2str("SP: %s\n", sway->on_shortest_path ? "YES" : "NO");
        }
        string shortestpath_str;
        if (!mo->sp_report.empty()) {
            shortestpath_str = "== Shortest Path ==\n" + mo->sp_report;
        }
        string query_str;
        if (!mo->query_report.empty()) {
            query_str = "== Query ==\n" + mo->query_report;
        }
        
        string str_to_print = 
            "== Last Frame ==\n" + last_redraw_str +
            "== This Frame ==\n" + redraw_str +
            snode_str + sway_str + query_str + shortestpath_str;
        if (!msg.empty()) str_to_print.append("== Message ==\n" + msg);
        print_string(str_to_print.c_str());
    }
    
    glFinish();
    if (use_double_buffer) glutSwapBuffers(); else glFlush();
    
    ed_clock = myclock();
            
    // calculate fps
    double cur_redraw_time = ed_clock - st_clock;
    double cur_refresh_time = cur_redraw_time + last_operation_time;
    const int refresh_time_list_avg = 10;
    if (refresh_time.size() >= refresh_time_list_avg)
        refresh_time.erase(refresh_time.begin());
    refresh_time.push_back(cur_refresh_time);
    double avg = 0;
    for (vector<double>::iterator it = refresh_time.begin(); it != refresh_time.end(); it++)
        avg += *it;
    avg /= refresh_time.size();
    double fps = 1000 / avg;
    last_redraw_str = printf2str("Average FPS: %.1f\nRedraw: %.2f ms\n", fps, cur_redraw_time);
    last_operation_time = 0;
}

void MapGraphics::set_kbd_specialkey()
{
    int mkey = glutGetModifiers();
    kbd_shift = mkey & GLUT_ACTIVE_SHIFT;
}
void MapGraphics::special_keyevent(int key, int x, int y)
{
    set_mouse_coord(x, y);
    MapOperation::MapOperationCode op;
    set_kbd_specialkey();
    switch (key) {
        case GLUT_KEY_F1: op = MapOperation::RESET_VIEW; break;
        case GLUT_KEY_F2: show_rtree ^= 1; op = MapOperation::NOP; break;
        case GLUT_KEY_F3: op = MapOperation::QUERY_NAME; break;
        case GLUT_KEY_F4: op = MapOperation::QUERY_TAG; break;
        case GLUT_KEY_F5: op = MapOperation::RUN_SHORTESTPATH; break;
        case GLUT_KEY_F6: show_shortestpath_node ^= 1; op = MapOperation::NOP; break;
        case GLUT_KEY_F7: op = MapOperation::SHOW_SHORTESTPATH_RESULT; break;
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
        default: printd("unknown key %d\n", key); return;
    }
    map_operation(op);
}

void MapGraphics::keyevent(unsigned char key, int x, int y)
{
    set_mouse_coord(x, y);
    MapOperation::MapOperationCode op;
    char kbd_char = key;
    if (isupper(kbd_char)) kbd_char = kbd_char - 'A' + 'a';
    set_kbd_specialkey();
    switch (kbd_char) { // convert SHIFT+NUM to NUM
        case '!': kbd_char = '1'; break;
        case '@': kbd_char = '2'; break;
        case '#': kbd_char = '3'; break;
        case '$': kbd_char = '4'; break;
        case '%': kbd_char = '5'; break;
        case '^': kbd_char = '6'; break;
        case '&': kbd_char = '7'; break;
        case '*': kbd_char = '8'; break;
        case '(': kbd_char = '9'; break;
        case ')': kbd_char = '0'; break;
    }
    if (isdigit(kbd_char)) {
        kbd_num = kbd_char == '0' ? 9 : kbd_char - '1';
        assert(kbd_num < MapOperation::MAX_KBDNUM);
        if (kbd_shift)
            op = MapOperation::NUMBER_POINT;
        else
            op = MapOperation::CENTER_NUM_POINT;
    } else if (kbd_char && strchr("qwertyuiop", kbd_char)) {
        const char *s = "qwertyuiop";
        kbd_num = strchr(s, kbd_char) - s;
        assert(kbd_num < MapOperation::MAX_KBDNUM);
        if (kbd_shift)
            op = MapOperation::NUMBER_WAY;
        else
            op = MapOperation::CENTER_NUM_WAY;
    } else switch (kbd_char) {
        case '-': case '_' : op = MapOperation::ZOOM_OUT; break;
        case '+': case '=' : op = MapOperation::ZOOM_IN; break;
        case 'z': op = MapOperation::ADD_POLYVERTEX; break;
        case 'x': op = MapOperation::CLEAR_POLYVERTEX; break;
        case 'k': op = MapOperation::MOVE_UP; break;
        case 'j': op = MapOperation::MOVE_DOWN; break;
        case 'h': op = MapOperation::MOVE_LEFT; break;
        case 'l': op = MapOperation::MOVE_RIGHT; break;
        case '\b': op = MapOperation::POP_DISPLAY; break;
        case '\x1b': op = MapOperation::CLEAR_ALL; break;
        case 'a': op = MapOperation::SET_SHORTESTPATH_START; break;
        case 's': op = MapOperation::SET_SHORTESTPATH_END; break;
        case 'd': op = MapOperation::CLEAR_SHORTESTPATH_VERTEX; break;
        case '\t': select_flag ^= 1; op = MapOperation::NOP; break;
        case '`': case '~':  op = MapOperation::SWITCH_SHORTESTPATH_ALGO; break;
        case '\\': case '|': show_message ^= 1; op = MapOperation::NOP; break;
        default: printd("unknown key %c\n", key); return;
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
    set_kbd_specialkey();
    set_mouse_coord(x, y);
    if (use_last_op) {
        op = last_mouse_op;
    } else if (select_flag && button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        op = MapOperation::SELECT_WAY;
    } else if (select_flag && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        op = MapOperation::SELECT_POINT;
    } else if (!select_flag && (button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON) && state == GLUT_DOWN) {
        if (!use_last_op) { // map dragging start
            last_mx = mx; last_my = my;
            is_dragging_map = true;
        }
        op = MapOperation::NOP;
    } else if (button == mouse_btn_zoomin && state == GLUT_DOWN) {
        op = MapOperation::ZOOM_IN_BY_MOUSE;
    } else if (button == mouse_btn_zoomout && state == GLUT_DOWN) {
        op = MapOperation::ZOOM_OUT_BY_MOUSE;
    } else if (state == GLUT_UP && is_dragging_map) {
        is_dragging_map = false; // map dragging stopped
        double diffmx = last_mx - mx;
        double diffmy = last_my - my;
        dminx += diffmx; dminy += diffmy;
        dmaxx += diffmx; dmaxy += diffmy;
        op = MapOperation::NOP;
    } else {
        last_mouse_op = MapOperation::NOP;
        printd("unknown mouse event %d %d %d\n", button, state, !!kbd_shift);
        return;
    }
    last_mouse_op = op;
    map_operation(op);
}

void MapGraphics::show_loading_screen()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    print_string("Loading data, please wait ...");
    glFinish();
    if (use_double_buffer) glutSwapBuffers(); else glFlush();
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
static void redraw_wrapper() { assert(mgptr); mgptr->redraw(); }
//static void redraw_wrapper() { assert(mgptr); TIMING("redraw", { mgptr->redraw(); }) }
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
    show_shortestpath_node = 0;
    last_operation_time = 0;
    select_flag = 0;
    is_dragging_map = false;
    show_message = 1;
    mo->init();
    window_width = initial_window_height * md->map_ratio;
    window_height = initial_window_height;
    reset_display_range();
    
    tlvl = md->ml.get_level_count();
    printd("tvil = %d\n", tlvl);
    vbuffer.resize(tlvl);
    ibuffer.resize(tlvl);
    vct_tvil.resize(tlvl);
    vct_vl.resize(tlvl);
    vct_il.resize(tlvl);
    vct_tl.resize(tlvl);
    
    /* glut things */
    glutInit(&argc, argv);
    unsigned int mode = GLUT_RGB;
    mode |= use_double_buffer ? GLUT_DOUBLE : GLUT_SINGLE;
    if (multisample_level) {
        glutSetOption(GLUT_MULTISAMPLE, multisample_level);
        mode |= GLUT_MULTISAMPLE;
    }
    glutInitDisplayMode(mode);
    glutInitWindowSize(window_width, window_height);
    glutCreateWindow(title);
    
    GLenum glewerr = glewInit();
    if (glewerr != GLEW_OK) {
        fail("glewInit() failed : %s", glewGetErrorString(glewerr));
    }
    if (!(GLEW_VERSION_1_5)) {
        const char *glstr;
        printf("opengl information:\n");
        glstr = (const char *) glGetString(GL_VENDOR); printf(" vendor: %s\n", glstr ? glstr : "null");
        glstr = (const char *) glGetString(GL_RENDERER); printf(" renderer: %s\n", glstr ? glstr : "null");
        glstr = (const char *) glGetString(GL_VERSION); printf(" version: %s\n", glstr ? glstr : "null");
        glstr = (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION); printf(" shading: %s\n", glstr ? glstr : "null");
        fail("opengl version is too low to run this program. (opengl 1.5 required)");
    }

    glutSpecialFunc(special_keyevent_wrapper);
    glutKeyboardFunc(keyevent_wrapper);
    glutMouseFunc(mouseupdown_wrapper);
    glutMotionFunc(mousemotion_wrapper);
    glutDisplayFunc(redraw_wrapper);
    glutReshapeFunc(reshape_wrapper);
    //glutIdleFunc(redraw_wrapper);
    
    if (use_double_buffer) show_loading_screen();
    show_loading_screen();
    
    glGenBuffers(tlvl, &vbuffer[0]);
    glGenBuffers(tlvl, &ibuffer[0]);
    if (!use_rtree_for_drawing) {
        timing_start("load vertex data");
        for (clvl = 0; clvl < tlvl; clvl++) {
            reload_dll(md->minx, md->maxx, md->miny, md->maxy);
            reload_dwl();
            reload_vertex();
        }
        dwl.clear();
        dll.clear();
        timing_end();
    }
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    
    msg = "Hello World!\nWelcome to ZBY's Map\n";

    const char *glstr;
    glstr = (const char *) glGetString(GL_VENDOR); if (glstr) { msg += "Vendor: "; msg += glstr; msg += '\n';}
    glstr = (const char *) glGetString(GL_RENDERER); if (glstr) { msg += "Renderer: "; msg += glstr; msg += '\n';}
    glstr = (const char *) glGetString(GL_VERSION); if (glstr) { msg += "Version: "; msg += glstr; msg += '\n';}
    glstr = (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION); if (glstr) { msg += "Shading: "; msg += glstr; msg += '\n'; }
    
    if (multisample_level) {
        glEnable(GL_MULTISAMPLE);
        int real_multisample_level;
        glGetIntegerv(GL_SAMPLES, &real_multisample_level);
        if (real_multisample_level != multisample_level) {
            printf("warning: %dx MSAA is not supported.\n", multisample_level);
        }
    }
    
    glutMainLoop(); // never return
    assert(0);
}

