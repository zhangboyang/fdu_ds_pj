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
    xmlldr.target(&md);
    xmlldr.load(cfgp.query("MAPDATA"));    
    
    // load level configurations
    int tot_lvl;
    tot_lvl = str2double(cfgp.query("LEVEL_TOTAL"));
    md.dfactor = str2double(cfgp.query("DISPLAY_FACTOR"));
    for (int lvl = 0; lvl < tot_lvl; lvl++) {
        char buf[MAXLINE];
        double res;
        sprintf(buf, "LEVEL_RES%d", lvl);
        res = str2double(cfgp.query(buf));
        md.ml.add_level(res);
    }
    
    md.construct();
    md.print_stat();
    
    //HeapProfilerStop();
    
    mg.target(&md);
    mg.show(cfgp.query("TITLE"), argc, argv); // ui loop, never return
    
    assert(0);
    return 0;
}
