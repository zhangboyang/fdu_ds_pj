#include "common.h"
#include "MapLevel.h"
#include <cstdio>
int MapLevel::get_level_count() { return lvllist.size(); }

void MapLevel::add_level(double res)
{
    // level 0 -> most detail, smallest resolution value
    // level MAXLEVEL-1 -> least detail, largest resolution value
    assert(lvllist.empty() || res >= lvllist.back());
    lvllist.push_back(res);
}

int MapLevel::select_level(double res)
{
    // convert current display resolution to display level
    // it's low limit, i.e. should display [lvl, MAXLEVEL)
    int lvl;
    for (lvl = 0; lvl < ((int) lvllist.size()) - 1; lvl++)
        if (lvllist[lvl + 1] >= res)
            break;
    return lvl;
}

double MapLevel::get_level(int lvl) { assert(0 <= lvl && lvl < (LL) lvllist.size()); return lvllist[lvl]; }

