#include <iostream>
#include "config.h"


using namespace std;

ConfigFilePraser cfgp;

int main()
{
    cfgp.load("config.txt");
    cout << cfgp.query("MAPDATA") << endl;
    return 0;
}
