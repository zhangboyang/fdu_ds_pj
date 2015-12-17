#include <cstdio>
#include <cstring>
#include "common.h"
#include "config.h"

#include <map>
#include <string>
using namespace std;

void ConfigFilePraser::load(const char *fn)
{
    /* c-style reading and prasing */
    FILE *f = fopen(fn, "r");
    if (!f) fail("can't open config file %s", fn);
    
    char buf[MAXLINE];
    while (fgets(buf, sizeof(buf), f)) {
        int len = strlen(buf);
        if (buf[len - 1] == '\n') buf[--len] = '\0';
        if (len == 0) continue;
        if (*buf == ';' || *buf == '#') continue;
        if (strncmp(buf, "//", 2) == 0) continue;
        char *ptr = strchr(buf, '=');
        char *val = ptr + 1;
        if (!ptr) fail("can't parse config file %s", fn);
        while (ptr > buf && *(ptr - 1) == ' ') ptr--;
        *ptr = '\0';
        //printd("key='%s' val='%s'\n", buf, ptr);
        if (!data.insert(make_pair(string(buf), string(val))).second)
            fail("duplicate config key %s\n", buf);
        not_queried_keys.insert(string(buf));
    }
}

const string &ConfigFilePraser::query(const string &key)
{
    assert(!data.empty());
    map<string, string>::iterator it;
    it = data.find(key);
    if (it == data.end()) fail("key %s not found", key.c_str());
    not_queried_keys.erase(key);
    return it->second;
}

const char *ConfigFilePraser::query(const char *key)
{
    return query(string(key)).c_str();
}

void ConfigFilePraser::check_not_queried_keys()
{
    for (std::set<std::string>::iterator it = not_queried_keys.begin(); it != not_queried_keys.end(); it++)
        printf("warning: not used config key %s\n", it->c_str());
}
