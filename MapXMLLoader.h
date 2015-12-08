#ifndef ZBY_XMLLOADER_H
#define ZBY_XMLLOADER_H
#include "common.h"
#include "MapObject.h"
#include "MapData.h"
#include <tinyxml2.h>

class MapXMLLoader {
    private:
    void print_xml(tinyxml2::XMLNode *node, int tab = 1);
    const char *query_attr(tinyxml2::XMLNode *node, const char *name, bool safe = false);
    void process_xmlchild(tinyxml2::XMLNode *node);
    void process_xmldoc(tinyxml2::XMLNode *doc);

    public:
    MapData *md;
    
    void target(MapData *md);
    void load(const char *fn);
};

#endif
