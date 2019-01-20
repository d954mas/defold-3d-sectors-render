#include <dmsdk/sdk.h>
#include "world.h"

#define TABLE_NAME "map"

void MapClear(){
    WORLD.reset();
}

void MapVertexAdd(float x, float y){
    WORLD.vertices.push_back(vec2f(x,y));
}

void MapVertexChange(int idx,float x, float y){
    if (idx <WORLD.vertices.size()){
        vec2f v = WORLD.vertices[idx];
        v.x = x; v.y = y;
    }else{
        dmLogError("no vertex with idx:%d", idx);
    }
}

void MapSectorCreate(float floor, float ceil){
    struct Sector s;
    s.id =  WORLD.sectors.size();
    s.floor = floor; s.ceil = ceil;
    WORLD.sectors.push_back(s);
}
//add to last sector in list
void MapSectorVertexAdd(int vertex,int neighbor){
    if (WORLD.sectors.size() != 0){
        Sector &s = WORLD.sectors[WORLD.sectors.size()-1];
        if (vertex >= 0 && vertex <WORLD.vertices.size()){ s.vertex.push_back(vertex);}
        else{ dmLogError("no vertex with idx:%d", vertex);return;}
        //todo check neighbors sector;
        s.neighbors.push_back(neighbor);
    }else{
        dmLogError("can't add vertex.No sectors");
    }
};

void MapCheck(){
    for(int i=0;i<WORLD.sectors.size();i++){
        Sector &s = WORLD.sectors[i];
        if(s.neighbors.size()<3){
            dmLogError("bad sector:%d", i);
            return;
        }
        for(int j=0;j<s.neighbors.size();j++){
            int n = s.neighbors[j];
            if (n!= -1 && n >= WORLD.sectors.size()){
                dmLogError("no neighbor with idx:%d", n);
            }
        }
    }
}

//bind


//region Map
static int MapClearLua(lua_State* L){
    MapClear();
    return 0;
}
static int MapVertexAddLua(lua_State* L){
    float x = luaL_checknumber(L, 1), y = lua_tonumber(L, 2);
    MapVertexAdd(x,y);
    return 0;
}
static int MapVertexChangeLua(lua_State* L){
    int idx = (int)luaL_checknumber(L, 1);
    float x = luaL_checknumber(L, 2), y = luaL_checknumber(L, 3);
    MapVertexChange(idx,x,y);
    return 0;
}
static int MapSectorCreateLua(lua_State* L){
	float floor = luaL_checknumber(L, 1), ceil = luaL_checknumber(L, 2);
    MapSectorCreate(floor,ceil);
    return 0;
}
//add to last sector in list
static int MapSectorVertexAddLua(lua_State* L){
    int vertex = (int)luaL_checknumber(L, 1), neigbor = (int)luaL_checknumber(L, 2);
    MapSectorVertexAdd(vertex,neigbor);
    return 0;
}
//check that map valid
static int MapCheckLua(lua_State* L){
    MapCheck();
    return 0;
}

static const luaL_reg Meta_methods[] = {
   	{"clear", MapClearLua},
   	{"vertex_add", MapVertexAddLua},
   	{"vertex_change", MapVertexChangeLua},
   	{"sector_create", MapSectorCreateLua},
   	{"sector_vertex_add", MapSectorVertexAddLua},
   	{"check", MapCheckLua},
    {0,0}
};


void MapBind(lua_State* L){
    int top = lua_gettop(L);
    lua_pushstring(L, TABLE_NAME);
    lua_newtable(L);
    luaL_register(L, NULL, Meta_methods);
    lua_settable(L, -3);
    assert(top == lua_gettop(L));
}



