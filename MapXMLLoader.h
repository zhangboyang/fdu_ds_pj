#ifndef ZBY_XMLLOADER_H
#define ZBY_XMLLOADER_H
#include "common.h"
#include "MapObject.h"
#include "MapData.h"
#include <libxml/xmlreader.h>

class MapXMLLoader {
    private:
    xmlTextReaderPtr rdr;
    MapWay *mway_ptr; // temp pointer used by process_node()
    
    void print_current_node();
    LL get_LL_attr(const char *aname);
    double get_double_attr(const char *aname);
    char *get_string_attr(const char *aname, char *buf, int size);
    void process_node();
    
    public:
    MapData *md;
    
    void target(MapData *md);
    void load(const char *fn);
};

#endif
