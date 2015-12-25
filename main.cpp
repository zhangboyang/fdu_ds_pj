#include <iostream>
#include "common.h"
#include "config.h"
#include "MapXMLLoader.h"
#include "MapData.h"
#include "MapGraphics.h"
#include "MapOperation.h"
#include "MapGUI.h"
#include "str2type.h"
#include "wstr.h"

#ifdef ZBY_OS_LINUX
//#define ENABLE_HEAP_PROFILE
#define ENABLE_CPU_PROFILE
#endif

#ifdef ENABLE_CPU_PROFILE
#include <google/profiler.h>
#endif
#ifdef ENABLE_HEAP_PROFILE
#include <google/heap-profiler.h>
#endif

static ConfigFilePraser cfgp;
static MapData md;
static MapXMLLoader xmlldr;
static MapGraphics mg;
static MapGUI mgui;
static MapOperation mo;
static MapShortestPath msp;

int main(int argc, char *argv[])
{
    #ifdef ZBY_OS_LINUX
    setlocale(LC_ALL, "zh_CN.UTF-8");
    #endif
    
    #ifdef ENABLE_HEAP_PROFILE
    HeapProfilerStart("pj_heap");
    #endif
    
    cfgp.load("config.txt");
    
    // load level configurations
    int tot_lvl;
    tot_lvl = str2LL(cfgp.query("LEVEL_TOTAL"));
    md.dfactor = str2double(cfgp.query("DISPLAY_FACTOR"));
    md.line_detail_dist_low_limit_factor = str2double(cfgp.query("LINE_DETAIL_DIST_LOW_LIMIT_FACTOR"));
    md.line_detail_dist_high_limit_factor = str2double(cfgp.query("LINE_DETAIL_DIST_HIGH_LIMIT_FACTOR"));
    md.line_detail_angle_limit = str2double(cfgp.query("LINE_DETAIL_ANGLE_LIMIT"));
    
    for (int lvl = 0; lvl < tot_lvl; lvl++) {
        char buf[MAXLINE];
        double res;
        sprintf(buf, "LEVEL_RES%d", lvl);
        res = str2double(cfgp.query(buf));
        md.ml.add_level(res);
    }
    
    // load waytype configurations
    int tot_waytype;
    tot_waytype = str2LL(cfgp.query("WAYTYPE_TOTAL"));
    for (int tid = 0; tid < tot_waytype; tid++) {
        char buf[MAXLINE];
        sprintf(buf, "WAYTYPE_TYPE%d", tid);
        md.wt.insert(cfgp.query(buf));
    }
    
    // load maptag configurations
    int tot_maptag;
    tot_maptag = str2LL(cfgp.query("MAPTAG_TOTAL"));
    for (int tid = 0; tid < tot_maptag; tid++) {
        char buf[MAXLINE];
        sprintf(buf, "MAPTAG%d", tid);
        md.mt.insert(cfgp.query(buf));
    }
    
    md.prepare();
    
    xmlldr.target(&md);
    xmlldr.load(cfgp.query("MAPDATA"));
    
    #ifdef ENABLE_HEAP_PROFILE
    HeapProfilerDump("xml file loaded");
    #endif
    
    md.construct();
    md.print_stat();
    
    
    
    // load graphics configurations
    mg.initial_window_height = str2double(cfgp.query("INITIAL_WINDOW_HEIGHT"));
    mg.move_step = str2double(cfgp.query("MOVE_STEP"));
    mg.zoom_step = str2double(cfgp.query("ZOOM_STEP"));
    mg.zoom_bysize_factor = str2double(cfgp.query("ZOOM_BYSIZE_FACTOR"));
    mg.use_rtree_for_drawing = str2LL(cfgp.query("USE_RTREE_FOR_DRAWING"));
    mg.use_double_buffer = str2LL(cfgp.query("USE_DOUBLE_BUFFER"));
    mg.multisample_level = str2LL(cfgp.query("MULTISAMPLE_LEVEL"));
    mg.mouse_btn_zoomin = str2LL(cfgp.query("MOUSE_BUTTON_ZOOMIN"));
    mg.mouse_btn_zoomout = str2LL(cfgp.query("MOUSE_BUTTON_ZOOMOUT"));
    mg.zoom_low_limit = str2double(cfgp.query("ZOOM_LOW_LIMIT"));
    mg.zoom_high_limit = str2double(cfgp.query("ZOOM_HIGH_LIMIT"));
    
    // load graphics color and thickness
    if (sscanf(cfgp.query("SELECTED_COLOR"), "%f | %f | %f", // selected colors
                 &mg.scolor[0], &mg.scolor[1], &mg.scolor[2]) != 3)
        fail("can't parse scolor");
    mg.selected_point_rect_size = str2double(cfgp.query("SELECTED_POINT_RECT_SIZE"));
    mg.selected_point_rect_thick = str2double(cfgp.query("SELECTED_POINT_RECT_THICK"));
    mg.selected_way_thick = str2double(cfgp.query("SELECTED_WAY_THICK"));
    for (int num = 0; num < MapOperation::MAX_KBDNUM; num++) {
        char buf[MAXLINE];
        sprintf(buf, "SELECTED_COLOR_NUM%d", num);
        if (sscanf(cfgp.query(buf), "%f | %f | %f",
                 &mg.ncolor[num][0], &mg.ncolor[num][1], &mg.ncolor[num][2]) != 3)
            fail("can't parse %s", buf);
    }
    if (sscanf(cfgp.query("POLY_COLOR"), "%f | %f | %f", // poly colors
                 &mg.pcolor[0], &mg.pcolor[1], &mg.pcolor[2]) != 3)
        fail("can't parse pcolor");
    mg.pthickness = str2double(cfgp.query("POLY_THICK"));
    if (sscanf(cfgp.query("NODE_RESULT_COLOR"), "%f | %f | %f", // node result colors
                 &mg.nrcolor[0], &mg.nrcolor[1], &mg.nrcolor[2]) != 3)
        fail("can't parse nrcolor");
    mg.nrsize = str2double(cfgp.query("NODE_RESULT_RECT_SIZE"));
    mg.nrthick = str2double(cfgp.query("NODE_RESULT_RECT_THICK"));
    if (sscanf(cfgp.query("WAY_RESULT_COLOR"), "%f | %f | %f", // way result colors
                 &mg.wrcolor[0], &mg.wrcolor[1], &mg.wrcolor[2]) != 3)
        fail("can't parse wrcolor");
    mg.wrthick = str2double(cfgp.query("WAY_RESULT_THICK"));
    mg.sp_vertex_rect_size = str2double(cfgp.query("SHORTESTPATH_VERTEX_RECT_SIZE")); // shortest path colors
    mg.sp_vertex_rect_thick = str2double(cfgp.query("SHORTESTPATH_VERTEX_RECT_THICK"));
    if (sscanf(cfgp.query("SHORTESTPATH_SRC_COLOR"), "%f | %f | %f",
                 &mg.sp_src_color[0], &mg.sp_src_color[1], &mg.sp_src_color[2]) != 3)
        fail("can't parse sp_src_color");
    if (sscanf(cfgp.query("SHORTESTPATH_DEST_COLOR"), "%f | %f | %f",
                 &mg.sp_dest_color[0], &mg.sp_dest_color[1], &mg.sp_dest_color[2]) != 3)
        fail("can't parse sp_dest_color");
    if (sscanf(cfgp.query("SHORTESTPATH_PATH_COLOR"), "%f | %f | %f",
                 &mg.sp_path_color[0], &mg.sp_path_color[1], &mg.sp_path_color[2]) != 3)
        fail("can't parse sp_path_color");
    mg.sp_path_thick = str2double(cfgp.query("SHORTESTPATH_PATH_THICK"));

    
    mg.target(&md);
    mg.target_gui(&mgui);
    mg.target_shortestpath(&msp);
    mg.target_operation(&mo);
    
    const char *window_title = cfgp.query("TITLE");
    cfgp.check_not_queried_keys();
    
    #ifdef ENABLE_HEAP_PROFILE
    HeapProfilerDump("construct finished");
    HeapProfilerStop();
    #endif
    #ifdef ENABLE_CPU_PROFILE
    ProfilerStart("pj_cpu");
    #endif
    
    mg.show(window_title, argc, argv); // ui loop, never return
    
    assert(0);
    return 0;
}
