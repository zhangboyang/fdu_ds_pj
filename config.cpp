#include <cstdio>
#include <cstring>
#include "common.h"
#include "config.h"

#include <map>
#include <string>
using namespace std;

#define MAXLINE 4096

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
        char *ptr = strchr(buf, '=');
        if (!ptr) fail("can't parse config file %s", fn);
        *ptr++ = '\0';
        printd("key='%s' val='%s'\n", buf, ptr);
        data.insert(make_pair(string(buf), string(ptr)));
    }
}

const string &ConfigFilePraser::query(const string &key)
{
    assert(!data.empty());
    map<string, string>::iterator it;
    it = data.find(key);
    if (it == data.end()) fail("key %s not found", key.c_str());
    return it->second;
}

const char *ConfigFilePraser::query(const char *key)
{
    return query(string(key)).c_str();
}

