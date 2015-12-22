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

//#define ENABLE_HEAP_PROFILE
#define ENABLE_CPU_PROFILE

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
    HeapProfilerStop();
    #endif
    
    md.construct();
    md.print_stat();
    
    
    
    // load graphics configurations
    mg.initial_window_height = str2double(cfgp.query("INITIAL_WINDOW_HEIGHT"));
    mg.move_step = str2double(cfgp.query("MOVE_STEP"));
    mg.zoom_step = str2double(cfgp.query("ZOOM_STEP"));
    mg.zoom_bysize_factor = str2double(cfgp.query("ZOOM_BYSIZE_FACTOR"));
    if (sscanf(cfgp.query("SELECTED_COLOR"), "%f | %f | %f",
                 &mg.scolor[0], &mg.scolor[1], &mg.scolor[2]) != 3)
        fail("can't parse scolor");
    mg.selected_point_rect_size = str2double(cfgp.query("SELECTED_POINT_RECT_SIZE"));
    mg.selected_point_rect_thick = str2double(cfgp.query("SELECTED_POINT_RECT_THICK"));
    mg.selected_way_thick = str2double(cfgp.query("SELECTED_WAY_THICK"));
    if (sscanf(cfgp.query("POLY_COLOR"), "%f | %f | %f",
                 &mg.pcolor[0], &mg.pcolor[1], &mg.pcolor[2]) != 3)
        fail("can't parse pcolor");
    mg.pthickness = str2double(cfgp.query("POLY_THICK"));
    if (sscanf(cfgp.query("NODE_RESULT_COLOR"), "%f | %f | %f",
                 &mg.nrcolor[0], &mg.nrcolor[1], &mg.nrcolor[2]) != 3)
        fail("can't parse nrcolor");
    mg.nrsize = str2double(cfgp.query("NODE_RESULT_RECT_SIZE"));
    mg.nrthick = str2double(cfgp.query("NODE_RESULT_RECT_THICK"));
    if (sscanf(cfgp.query("WAY_RESULT_COLOR"), "%f | %f | %f",
                 &mg.wrcolor[0], &mg.wrcolor[1], &mg.wrcolor[2]) != 3)
        fail("can't parse wrcolor");
    mg.wrthick = str2double(cfgp.query("WAY_RESULT_THICK"));
    for (int num = 0; num < MapOperation::MAX_KBDNUM; num++) {
        char buf[MAXLINE];
        sprintf(buf, "SELECTED_COLOR_NUM%d", num);
        if (sscanf(cfgp.query(buf), "%f | %f | %f",
                 &mg.ncolor[num][0], &mg.ncolor[num][1], &mg.ncolor[num][2]) != 3)
            fail("can't parse %s", buf);
    }
    
    mg.target(&md);
    mg.target_gui(&mgui);
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
    
    //fclose(stdout);
    mg.show(window_title, argc, argv); // ui loop, never return
    
    assert(0);
    return 0;
}
