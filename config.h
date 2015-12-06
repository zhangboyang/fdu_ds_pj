#ifndef ZBY_CONFIG_H
#define ZBY_CONFIG_H

#include <map>
#include <string>

class ConfigFilePraser {
    private:
    std::map<std::string, std::string> data;
    public:
    void load(const char *fn);
    const std::string &query(const std::string &key);
    const char *query(const char *key);
};

#endif
