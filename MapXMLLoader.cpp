#include <tinyxml2.h>
#include "common.h"
#include "MapXMLLoader.h"
#include "str2type.h"

using namespace tinyxml2;
/* use tinyxml2 to read data */

void MapXMLLoader::print_xml(XMLNode *node, int tab)
{
    printf("%*s", tab + 1, "+");
    XMLElement *ele;
    if ((ele = node->ToElement())) {
        printf("element=%s\n", node->Value());
        const XMLAttribute *attr = ele->FirstAttribute();
        while (attr) {
	        int ival; double dval;
	        printf("%*s", tab + 2, "$");
		    printf("%s: value=[%s]", attr->Name(), attr->Value());
		    if (attr->QueryIntValue(&ival) == XML_NO_ERROR) printf(" int=%d", ival);
		    if (attr->QueryDoubleValue(&dval) == XML_NO_ERROR) printf(" d=%1.1f", dval);
		    printf("\n");
		    attr = attr->Next();
	    }
    } else if (node->ToText()) {
        printf("txt=%s\n", node->Value());
    } else {
        printf("unknown\n");
    }
    
    XMLNode *ch;
    for (ch = node->FirstChild(); ch; ch = ch->NextSibling()) {
        print_xml(ch, tab + 1);
    }
}

const char *MapXMLLoader::query_attr(XMLNode *node, const char *name, bool safe)
{
    XMLElement *ele = node->ToElement();
    assert(ele);
    const XMLAttribute *attr;
    for (attr = ele->FirstAttribute(); attr; attr = attr->Next())
        if (strcmp(name, attr->Name()) == 0)
            return attr->Value();
    if (!safe) {
        printd("query_attr(%p, %s)\n", node, name);
        print_xml(node);
        fail("attr '%s' not found", name);
    }
    return NULL;
}

void MapXMLLoader::process_xmlchild(XMLNode *node)
{
    const char *tstr = node->Value();    
    if (strcmp(tstr, "bounds") == 0) {
        md->set_coord_limit(str2double(query_attr(node, "minlat")),
                            str2double(query_attr(node, "maxlat")),
                            str2double(query_attr(node, "minlon")),
                            str2double(query_attr(node, "maxlon")));
    } else if (strcmp(tstr, "node") == 0) {
        LL id = str2LL(query_attr(node, "id"));
        MapNode *mnode = new MapNode;
        mnode->set_id(id);
        md->set_node_coord_by_geo(mnode, str2double(query_attr(node, "lat")),
                                         str2double(query_attr(node, "lon")));
        md->insert(mnode);
    } else if (strcmp(tstr, "way") == 0) { //print_xml(node);
        LL id = str2LL(query_attr(node, "id"));
        MapWay *mway = new MapWay;
        mway->set_id(id);
        XMLNode *ch;
        for (ch = node->FirstChild(); ch; ch = ch->NextSibling())
            if (strcmp(ch->Value(), "nd") == 0)
                mway->add_node(md->get_node_by_id(str2LL(query_attr(ch, "ref"))));
        md->insert(mway);
    } else if (strcmp(tstr, "relation") == 0) {
        static int x = 0; // FIXME
        if (!x) { printf("warning: relation ignored.\n"); x = 1; }
    } else {
        printf("warning: unknown node type %s\n", tstr);
        print_xml(node);
        return;
    }
}

void MapXMLLoader::process_xmldoc(XMLNode *doc)
{
    assert(doc->FirstChild());
    doc = doc->FirstChild()->NextSibling();
    assert(doc);
    assert(strcmp(doc->Value(), "osm") == 0);
    
    XMLNode *ch;
    for (ch = doc->FirstChild(); ch; ch = ch->NextSibling()) {
        process_xmlchild(ch);
    }
}

void MapXMLLoader::load(const char *xmlfile)
{
    assert(md);
    XMLDocument doc;
    
    TIMING ("load xml file", {
        if (doc.LoadFile(xmlfile) != XML_NO_ERROR) { // load data from file
            fail("doc.LoadFile() failed");
        }
    })
    
    TIMING ("process xml", {
        process_xmldoc(&doc); // process xml document
    })
    md->print_stat();
    assert(md->nl.size() == md->nm.size());
    assert(md->wl.size() == md->wm.size());
    assert(md->rl.size() == md->rm.size());
}

void MapXMLLoader::target(MapData *md) { MapXMLLoader::md = md; }
