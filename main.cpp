#include <iostream>
#include "common.h"
#include "config.h"
#include "MapXMLLoader.h"
#include "MapData.h"
#include "MapGraphics.h"

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
    //HeapProfilerStop();
    mg.target(&md);
    mg.show(cfgp.query("TITLE"), argc, argv); // ui loop, never return
    
    assert(0);
    return 0;
}
