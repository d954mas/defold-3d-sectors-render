#include <dmsdk/sdk.h>
#include "world.h"
#define TABLE_NAME "world"

void WorldUpdate(float dt){
     ECS.update(dt);
}

void World::reset(){
    sectors.clear();
    vertices.clear();
    ECS.reset();
}

World WORLD;
EcsWorld ECS;

//region BIND
static int WorldUpdateLua(lua_State* L){
    float dt = luaL_checknumber(L, 1);
    WorldUpdate(dt);
    return 0;
}

static const luaL_reg Meta_methods[] = {
   {"update", WorldUpdateLua},
    {0,0}
};

void WorldBind(lua_State *L){
    int top = lua_gettop(L);
    lua_pushstring(L, TABLE_NAME);
    lua_newtable(L);
    luaL_register(L, NULL, Meta_methods);
    lua_settable(L, -3);
    assert(top == lua_gettop(L));
}


//endregion
