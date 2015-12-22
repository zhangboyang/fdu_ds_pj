#include <cstring>
#include <libxml/xmlreader.h>
#include "common.h"
#include "MapXMLLoader.h"
#include "str2type.h"
#include "wstr.h"

using namespace std;

/* use libxml2 to read data */

void MapXMLLoader::print_current_node()
{
    int deepth = xmlTextReaderDepth(rdr);
    int tab = deepth + 1;
    int ntype = xmlTextReaderNodeType(rdr);
    const char *name = (const char *) xmlTextReaderConstName(rdr);
    const char *value = (const char *) xmlTextReaderConstValue(rdr);
    
    if (ntype != 1) {
        printf("%*stype=%d\n", tab, "+", ntype);
        return;
    }
    
    printf("%*stype=%d name=%s\n", tab, "+", ntype, name);
    
    int ret;
    ret = xmlTextReaderMoveToFirstAttribute(rdr);
    while (ret == 1) {
        name = (const char *) xmlTextReaderConstName(rdr);
        value = (const char *) xmlTextReaderConstValue(rdr);
        printf("%*skey=%s val=%s\n", tab + 1, "$", name, value);
        ret = xmlTextReaderMoveToNextAttribute(rdr);
    }
}

LL MapXMLLoader::get_LL_attr(const char *aname)
{
    char *str = (char *) xmlTextReaderGetAttribute(rdr, (xmlChar *) aname);
    if (!str) {
        print_current_node();
        fail("can't find attribute '%s'", aname);
    }
    LL ret = str2LL(str);
    xmlFree(str);
    return ret;
}

double MapXMLLoader::get_double_attr(const char *aname)
{
    char *str = (char *) xmlTextReaderGetAttribute(rdr, (xmlChar *) aname);
    if (!str) {
        print_current_node();
        fail("can't find attribute '%s'", aname);
    }
    double ret = str2double(str);
    xmlFree(str);
    return ret;
}

char *MapXMLLoader::get_string_attr(const char *aname, char *buf, int size)
{
    char *str = (char *) xmlTextReaderGetAttribute(rdr, (xmlChar *) aname);
    strncpy(buf, str, size);
    buf[size - 1] = '\0';
    xmlFree(str);
    return buf;
}

void MapXMLLoader::process_node()
{
    int deepth = xmlTextReaderDepth(rdr);
    int ntype = xmlTextReaderNodeType(rdr);
    
    if (ntype != 1) return;
    
    const char *name = (const char *) xmlTextReaderConstName(rdr);
    
    if (mway_ptr && deepth == 2) {
        // process child of 'way', see processing 'way' below
        if (strcmp(name, "nd") == 0)
            mway_ptr->add_node(md->get_node_by_id(get_LL_attr("ref")));
        else if (strcmp(name, "tag") == 0) {
            char keybuf[MAXLINE];
            char valbuf[MAXLINE];
            get_string_attr("k", keybuf, sizeof(keybuf));
            get_string_attr("v", valbuf, sizeof(valbuf));
            mway_ptr->tl.push_back(make_pair(string(keybuf), s2ws(string(valbuf))));
            int tagid = md->mt.query(keybuf, valbuf);
            if (tagid >= 0) md->insert_with_tag(mway_ptr, tagid);
            if (strcmp(keybuf, "highway") == 0) {
                string t(keybuf);
                t += '/';
                t += valbuf;
                mway_ptr->waytype = md->wt.query_id(t);
                mway_ptr->on_shortest_path = true;
            }
            if (strcmp(keybuf, "oneway") == 0 && strcmp(valbuf, "yes") == 0) {
                mway_ptr->one_way = true;
            }
            if (md->tag_key_is_name(string(keybuf))) {
                wstring wstr(s2ws(string(valbuf)));
                md->wd.insert(wstr.c_str(), mway_ptr);
            }
        }
        return;
    }
    if (mnode_ptr && deepth == 2) {
        // process node names
        if (strcmp(name, "tag") == 0) {
            char keybuf[MAXLINE];
            char valbuf[MAXLINE];
            get_string_attr("k", keybuf, sizeof(keybuf));
            get_string_attr("v", valbuf, sizeof(valbuf));
            mnode_ptr->tl.push_back(make_pair(string(keybuf), s2ws(string(valbuf))));
            int tagid = md->mt.query(keybuf, valbuf);
            if (tagid >= 0) md->insert_with_tag(mnode_ptr, tagid);
            if (md->tag_key_is_name(string(keybuf))) {
                wstring wstr(s2ws(string(valbuf)));
                md->nd.insert(wstr.c_str(), mnode_ptr);
            }
        }
        return;
    }
    if (deepth != 1) return;
    
    mway_ptr = NULL; // reset temp ptr
    mnode_ptr = NULL;
    if (strcmp(name, "node") == 0) {
        LL id = get_LL_attr("id");
        MapNode *mnode = new MapNode;
        mnode->set_id(id);
        md->set_node_coord_by_geo(mnode, get_double_attr("lat"),
                                         get_double_attr("lon"));
        md->insert(mnode);
        mnode_ptr = mnode; // process node later
    } else if (strcmp(name, "way") == 0) {
        LL id = get_LL_attr("id");
        MapWay *mway = new MapWay;
        mway->set_id(id);
        md->insert(mway);
        mway_ptr = mway; // process child later
    } else if (strcmp(name, "relation") == 0) {
        static int x = 0; // FIXME: 'relation' not implemented
        if (!x) { printf("warning: relation ignored.\n"); x = 1; }
    } else if (strcmp(name, "bounds") == 0) {
        md->set_coord_limit(get_double_attr("minlat"),
                            get_double_attr("maxlat"),
                            get_double_attr("minlon"),
                            get_double_attr("maxlon"));
    } else {
        printf("warning: unknown node type %s\n", name);
        print_current_node();
        return;
    }
}

void MapXMLLoader::load(const char *fn)
{
    assert(md);
    
    LIBXML_TEST_VERSION
    
    // this function will load data to md->[nwr]l and md->[nwr]m
    // which will be processed lator by MapData::construct()
    
    timing_start("load data from xml");
        rdr = xmlReaderForFile(fn, NULL, 0);
        if (!rdr) fail("can't load '%s'", fn);
        mway_ptr = NULL; // reset temp ptr for process_node()
        mnode_ptr = NULL;
        int ret;
        ret = xmlTextReaderRead(rdr);
        while (ret == 1) {
            process_node();
            ret = xmlTextReaderRead(rdr);
        }
        xmlFreeTextReader(rdr);
        if (ret != 0) fail("failed to parse '%s'", fn);
    timing_end();
}

void MapXMLLoader::target(MapData *md) { MapXMLLoader::md = md; }
