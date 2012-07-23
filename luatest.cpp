#include <iostream>
#include <lua.hpp>
#include "otml.h"

/*
void push_otml_subnode_luavalue(const OTMLNodePtr& node)
{
    if(node->hasValue()) {
        // convert boolean types
        if(node->value() == "true" || node->value() == "false")
            g_lua.pushBoolean(node->value<bool>());
        else
            g_lua.pushString(node->value());
    } else if(node->hasChildren()) {
        g_lua.newTable();
        bool pushedChild = false;
        int currentIndex = 1;
        for(const OTMLNodePtr& cnode : node->children()) {
            push_otml_subnode_luavalue(cnode);
            if(!g_lua.isNil()) {
                if(cnode->isUnique()) {
                    g_lua.pushString(cnode->tag());
                    g_lua.insert(-2);
                    g_lua.rawSet();
                } else
                    g_lua.rawSeti(currentIndex++);
                pushedChild = true;
            } else
                g_lua.pop();
        }
        if(!pushedChild) {
            g_lua.pop();
            g_lua.pushNil();
        }
    } else
        g_lua.pushNil();
}

*/
void lua_pushOtmlValue(lua_State* L, const OTMLNodePtr& node)
{

    if(node->hasValue()) {
        union {
            bool b;
            double d;
            long l;
        };
        std::string value = node->rawValue();
        if(otml_util::cast(value, b))
            lua_pushboolean(L, b);
        else if(otml_util::cast(value, l))
            lua_pushinteger(L, l);
        else if(otml_util::cast(value, d))
            lua_pushnumber(L, d);
        else
            lua_pushstring(L, value.c_str());
    } else if(node->hasChildren()) {
        lua_newtable(L);
        bool pushedChild = false;
        int currentIndex = 1;
        OTMLNodeList children = node->children();
        for(OTMLNodeList::const_iterator it = children.begin(), end = children.end(); it != end; it ++) {
            const OTMLNodePtr& cnode = *it;
            lua_pushOtmlValue(L, cnode);

            if(!lua_isnil(L, -1)) {
                if(cnode->isUnique()) {
                    lua_pushstring(L, cnode->tag().c_str());
                    lua_insert(L, -2);
                    lua_rawset(L, -3);
                } else
                    lua_rawseti(L, -2, currentIndex++);

                pushedChild = true;
            } else
                lua_pop(L, 1);
        }
        if(!pushedChild) {
            lua_pop(L, 1);
            lua_pushnil(L);
        }
    } else
        lua_pushnil(L);
}

void lua_pushOtmlNode(lua_State* L, const OTMLNodePtr& node)
{
    if(node) {
        lua_newtable(L);
        int currentIndex = 1;
        OTMLNodeList children = node->children();
        for(OTMLNodeList::const_iterator it = children.begin(), end = children.end(); it != end; it ++) {
            const OTMLNodePtr& cnode = *it;

            lua_pushOtmlValue(L, cnode);
            if(cnode->isUnique() && !cnode->tag().empty()) {
                lua_setfield(L, -2, cnode->tag().c_str());
            } else
                lua_rawseti(L, -2, currentIndex++);
        }
    } else
        lua_pushnil(L);
}

int lua_loadOtml(lua_State* L)
{
    try {
        std::string file = lua_tostring(L, -1);
        lua_pop(L, 1);
        OTMLDocumentPtr doc = OTMLDocument::parse(file);
        lua_pushOtmlNode(L, doc);
    } catch(std::exception& e) {
        std::cerr << "lua_loadOtml ERROR: " << e.what() << std::endl;
        lua_pushnil(L);
    }
    return 1;
}

int main(int argc, char** argv)
{
    const char* file = "test.lua";
    if(argc > 1)
        file = argv[1];

    lua_State *L = lua_open();
    luaL_openlibs(L);

    lua_register(L, "loadOtml", lua_loadOtml);

    std::cout << "Running " << file << std::endl;

    int ret = luaL_loadfile(L, file);
    if(ret != 0) {
        std::cerr << "-- " << lua_tostring(L, -1) << std::endl;
        return -1;
    }

    ret = lua_pcall(L, 0, LUA_MULTRET, 0);

    if(ret != 0) {
        std::cerr << "ERROR: " << lua_tostring(L, -1) << std::endl;
        return -1;
    }

    lua_close(L);

    std::cout << "Done." << std::endl;

    return 0;
}
