#ifndef ZBY_MAPLEVEL_H
#define ZBY_MAPLEVEL_H
#include "common.h"

#include <vector>

class MapLevel {
    // convert real size to level
    private:
    std::vector<double> lvllist;
    public:
    int get_level_count();
    void add_level(double res);
    int select_level(double res);
    double get_level(int lvl);
};
#endif
