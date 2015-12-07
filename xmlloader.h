#ifndef ZBY_XMLLOADER_H
#define ZBY_XMLLOADER_H
#include "common.h"
#include "MapObject.h"
#include "MapData.h"
#include <tinyxml.h>

class MapXMLLoader {
    private:
    void print_xml(TiXmlNode *node, int tab = 1);
    const char *query_attr(TiXmlNode *node, const char *name, bool safe = false);
    void process_xmlchild(TiXmlNode *node);
    void process_xmldoc(TiXmlNode *doc);

    public:
    MapData *md;
    
    void target(MapData *md);
    void load(const char *fn);
};

#endif
