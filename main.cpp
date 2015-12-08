#include <iostream>
#include "common.h"
#include "config.h"
#include "MapXMLLoader.h"
#include "MapData.h"
#include "MapGraphics.h"

ConfigFilePraser cfgp;
MapData md;
MapXMLLoader xmlldr;
MapGraphics mg;

int main(int argc, char *argv[])
{
    cfgp.load("config.txt");
    xmlldr.target(&md);
    xmlldr.load(cfgp.query("MAPDATA"));
    mg.target(&md);
    mg.show(cfgp.query("TITLE"), argc, argv); // ui loop
    return 0;
}
