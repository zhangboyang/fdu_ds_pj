#ifndef ZBY_CONFIG_H
#define ZBY_CONFIG_H
#include "common.h"

#include <map>
#include <set>
#include <string>

class ConfigFilePraser {
    private:
    std::map<std::string, std::string> data;
    std::set<std::string> not_queried_keys;
    public:
    void load(const char *fn);
    const std::string &query(const std::string &key);
    const char *query(const char *key);
    void check_not_queried_keys();
};

#endif
