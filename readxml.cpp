#include <tinyxml.h>
#include "common.h"
#include "readxml.h"

/* use tinyxml to read data */

static void print_xml(TiXmlNode *node, int tab = 0)
{
    printf("%*s", tab + 1, "+");
    printf("type=%d ", node->Type());
    if (node->Type() == TiXmlNode::TINYXML_ELEMENT) {
        printf("node->val=%s\n", node->Value());
        TiXmlElement *ele = node->ToElement();
        TiXmlAttribute *attr = ele->FirstAttribute();
        while (attr)
	    {
	        int ival; double dval;
	        printf("%*s", tab + 1, "$");
		    printf("%s: value=[%s]", attr->Name(), attr->Value());
		    if (attr->QueryIntValue(&ival)==TIXML_SUCCESS) printf(" int=%d", ival);
		    if (attr->QueryDoubleValue(&dval)==TIXML_SUCCESS) printf(" d=%1.1f", dval);
		    printf("\n");
		    attr = attr->Next();
	    }
    }
    printf("\n");
    
    TiXmlNode *ch;
    for (ch = node->FirstChild(); ch != NULL; ch = ch->NextSibling()) {
        print_xml(ch, tab + 1);
    }
}
void load_xml(const char *xmlfile)
{
    TiXmlDocument doc;
    TIMING ("load xml", {
        if (!doc.LoadFile(xmlfile)) {
            fail("doc.LoadFile() failed");
        }
    })
    print_xml(&doc);
}

