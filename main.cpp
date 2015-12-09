#include <iostream>
#include "common.h"
#include "config.h"
#include "MapXMLLoader.h"
#include "MapData.h"
#include "MapGraphics.h"
#include "str2type.h"

//#include <google/heap-profiler.h>

ConfigFilePraser cfgp;
MapData md;
MapXMLLoader xmlldr;
MapGraphics mg;

int main(int argc, char *argv[])
{
    //HeapProfilerStart("pj_hp");
    cfgp.load("config.txt");
    // load level configurations
    int tot_lvl;
    tot_lvl = str2LL(cfgp.query("LEVEL_TOTAL"));
    md.dfactor = str2double(cfgp.query("DISPLAY_FACTOR"));
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
    
    
    
    xmlldr.target(&md);
    xmlldr.load(cfgp.query("MAPDATA"));    
    
    md.construct();
    md.print_stat();
    
    //HeapProfilerStop();
    
    mg.target(&md);
    mg.show(cfgp.query("TITLE"), argc, argv); // ui loop, never return
    
    assert(0);
    return 0;
}
