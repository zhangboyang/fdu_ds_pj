#include <cstdio>
#include <cstring>
#include <cctype>
#include "common.h"
#include "MapTaxiRoute.h"

#include <set>
#include <algorithm>
using namespace std;

void MapTaxiRoute::set_filename(const char *fn) { this->fn = fn; }

void MapTaxiRoute::preprocess() // scan file, calc offsets
{
    if (!*fn) { // if file name is empty, don't process taxi data
        printf("warning: taxi data not specified\n");
        return;
    }
    FILE *fp = fopen(fn, "r");
    if (!fp) fail("can't open taxi data file %s", fn);
    
    set<int> id_set;
    char buf[MAXLINE];
    int tot_line_cnt = 0;
    int *node_cnt_ptr;
    char taxi_id[MAXLINE];
    int taxi_id_len;
    int taxi_id_num;
    int flag = 1; // flag: try to read texi_id
    long offset;
    int ret;
    while (offset = ftell(fp), fgets(buf, sizeof(buf), fp)) {
        char *ptr = buf;
        while (*ptr && isspace(*ptr)) ptr++;
        
        if (*ptr) {
            if (flag) { // read new taxi id
                ret = sscanf(ptr, "%[^,]", taxi_id);
                if (ret != 1) {
                    printf("line: %d\ndata: %s\n", tot_line_cnt, ptr);
                    fail("can't read taxi id");
                }
                taxi_id_len = strlen(taxi_id);
                sscanf(taxi_id, "%d", &taxi_id_num);
                //printd("taxi_id=%s taxi_id_num=%d offset=%ld\n", taxi_id, taxi_id_num, offset);
                if (id_set.find(taxi_id_num) != id_set.end()) {
                    fail("duplicate taxi detected, id = %d, offset = %ld", taxi_id_num, offset);
                }
                id_set.insert(taxi_id_num);
                tl.push_back(make_pair(taxi_id_num, make_pair(1, offset)));
                node_cnt_ptr = &tl.back().second.first;
                flag = 0;
            } else { // compare taxi id for verify
                (*node_cnt_ptr)++;
                if (strncmp(ptr, taxi_id, taxi_id_len) != 0) {
                    printf("line: %d\ndata: %s\n", tot_line_cnt, ptr);
                    fail("taxi id mismatch");
                }
            }
        } else { // a empty line means a new taxi route
            flag = 1;
        }
        
        tot_line_cnt++;
    }
    
    sort(tl.begin(), tl.end());
    
    printf("taxi data statistics:\n");
    printf(" taxi count: %d\n", (int) tl.size());
    printf(" line count: %d\n", tot_line_cnt);

    fclose(fp);
}

int MapTaxiRoute::find_index_by_id(int taxi_id)
{
    int taxi_index;
    taxi_index = lower_bound(tl.begin(), tl.end(), make_pair(taxi_id, make_pair(0, 0l))) - tl.begin();
    if (taxi_index >= (int) tl.size() || tl[taxi_index].first != taxi_id) return -1;
    return taxi_index;
}

void MapTaxiRoute::load_route(int taxi_index)
{
    assert(0 <= taxi_index && taxi_index < (int) tl.size());
    int taxi_id = tl[taxi_index].first;
    cur_taxi_id = taxi_id;
    long offset = tl[taxi_index].second.second;
    
    printd("id = %d, offset = %ld\n", taxi_id, offset);
    
    FILE *fp = fopen(fn, "r");
    if (!fp) fail("can't open taxi data file %s", fn);
    
    if (fseek(fp, offset, SEEK_SET) != 0)
        fail("can't seek to requested offset %ld", offset);
    
    tnl.clear();
    char buf[MAXLINE];
    char trash[MAXLINE];
    while (fgets(buf, sizeof(buf), fp)) {
        taxi_node nd;
        
        char *ptr = buf;
        while (*ptr && isspace(*ptr)) ptr++;
        
        if (!*ptr) break; // if a blank line is found, it finished
        int ret = sscanf(ptr, "%d,%19[^,],%d,%lf,%lf,%d,%c,%d,%d,%f,%f,%d%s",
                &nd.taxi_id, nd.timestr, &nd.is_alert, &nd.lon, &nd.lat, &nd.is_empty,
                &nd.light_state, &nd.is_highway, &nd.is_brake, &nd.speed, &nd.direction,
                &nd.gps_count, trash);
        
        if (ret != 12 || strlen(nd.timestr) != 19) {
            printf("can't prase taxi data line:\n");
            printf("[%d,%s,%d,%f,%f,%d,%c,%d,%d,%f,%f,%d]\n",
                nd.taxi_id, nd.timestr, nd.is_alert, nd.lon, nd.lat, nd.is_empty,
                nd.light_state, nd.is_highway, nd.is_brake, nd.speed, nd.direction,
                nd.gps_count);
            printf("[%s]\n", ptr);
            fail("parse error");
        }
        
        assert(nd.taxi_id == taxi_id);
        nd.node_id = tnl.size();
        tnl.push_back(nd);
    }
    
    printd("load finished: id = %d, node count = %d, memory = %f mb\n",
        taxi_id, (int) tnl.size(), (tnl.size() * sizeof(taxi_node)) / 1048576.0);
    
    fclose(fp);
}
