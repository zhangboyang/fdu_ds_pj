#include <iostream>
#include "common.h"
#include "config.h"
#include "xmlloader.h"

using namespace std;

ConfigFilePraser cfgp;
MapData md;
MapXMLLoader xmlldr;


int main()
{
    cfgp.load("config.txt");
    xmlldr.target(&md);
    xmlldr.load(cfgp.query("MAPDATA"));
    
    return 0;
}
