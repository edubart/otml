#include <iostream>
#include "otml.h"

void testWrite(const std::string& filename)
{
    OTMLDocumentPtr doc = OTMLDocument::create();
    doc->writeAt("test field one", "hello");
    doc->writeAt("test field two", 1);

    OTMLNodePtr node = OTMLNode::create("sub node", true);
    node->writeAt("test field one", "world");
    node->writeAt("test field two", 2);
    doc->addChild(node);

    doc->save(filename);
}

void testRead(const std::string& filename)
{
    OTMLDocumentPtr doc = OTMLDocument::parse(filename);
    std::cout << doc->emit() << std::endl;
}

int main(int argc, char** argv)
{
    testWrite("test.otml");
    testRead("test.otml");
    return 0;
}
