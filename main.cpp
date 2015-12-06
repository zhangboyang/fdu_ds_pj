#include <iostream>
#include "common.h"
#include "config.h"
#include "readxml.h"

using namespace std;

ConfigFilePraser cfgp;

int main()
{
    cfgp.load("config.txt");
    load_xml(cfgp.query("MAPDATA"));
    return 0;
}
