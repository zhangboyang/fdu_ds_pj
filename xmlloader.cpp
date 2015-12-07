#include <tinyxml.h>
#include "common.h"
#include "xmlloader.h"
#include "str2type.h"

/* use tinyxml to read data */

void MapXMLLoader::print_xml(TiXmlNode *node, int tab)
{
    printf("%*s", tab + 1, "+");
    printf("type=%d ", node->Type());
    if (node->Type() == TiXmlNode::TINYXML_ELEMENT) {
        printf("node->val=%s\n", node->Value());
        TiXmlElement *ele = node->ToElement();
        TiXmlAttribute *attr = ele->FirstAttribute();
        while (attr) {
	        int ival; double dval;
	        printf("%*s", tab + 1, "$");
		    printf("%s: value=[%s]", attr->Name(), attr->Value());
		    if (attr->QueryIntValue(&ival)==TIXML_SUCCESS) printf(" int=%d", ival);
		    if (attr->QueryDoubleValue(&dval)==TIXML_SUCCESS) printf(" d=%1.1f", dval);
		    printf("\n");
		    attr = attr->Next();
	    }
    } else {
        printf("\n");
    }
    
    TiXmlNode *ch;
    for (ch = node->FirstChild(); ch; ch = ch->NextSibling()) {
        print_xml(ch, tab + 1);
    }
}

const char *MapXMLLoader::query_attr(TiXmlNode *node, const char *name, bool safe)
{
    assert(node->Type() == TiXmlNode::TINYXML_ELEMENT);
    TiXmlElement *ele = node->ToElement();
    TiXmlAttribute *attr;
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

void MapXMLLoader::process_xmlchild(TiXmlNode *node)
{
    const char *tstr = node->Value();    
    if (strcmp(tstr, "bounds") == 0) {
        md->set_coord_limit(str2double(query_attr(node, "minlat")),
                            str2double(query_attr(node, "maxlat")),
                            str2double(query_attr(node, "minlon")),
                            str2double(query_attr(node, "maxlon")));
    } else if (strcmp(tstr, "node") == 0) {
        LL id = str2LL(query_attr(node, "id"));
        double lat = str2double(query_attr(node, "lat"));
        double lon = str2double(query_attr(node, "lon"));
        MapNode *mnode = new MapNode;
        mnode->set_id(id);
        mnode->set_coord(lat, lon);
        md->insert(mnode);
    } else if (strcmp(tstr, "way") == 0) { //print_xml(node);
        LL id = str2LL(query_attr(node, "id"));
        MapWay *mway = new MapWay;
        mway->set_id(id);
        TiXmlNode *ch;
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

void MapXMLLoader::process_xmldoc(TiXmlNode *doc)
{
    assert(doc->FirstChild());
    doc = doc->FirstChild()->NextSibling();
    assert(doc);
    assert(strcmp(doc->Value(), "osm") == 0);
    
    
    TiXmlNode *ch;
    for (ch = doc->FirstChild(); ch; ch = ch->NextSibling()) {
        process_xmlchild(ch);
    }
}

void MapXMLLoader::load(const char *xmlfile)
{
    assert(md);
    TiXmlDocument doc;
    TIMING ("load xml file", {
        if (!doc.LoadFile(xmlfile)) { // load data from file
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
