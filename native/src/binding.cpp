// myextension.cpp
// Extension lib defines
#define EXTENSION_NAME Native
#define LIB_NAME "Native"
#define MODULE_NAME "native"

#include <math.h>
#include "3d_render.h"
#include "ecs_bind.h"
#include <dmsdk/sdk.h>

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

//endregion


//region World
static int WorldUpdateLua(lua_State* L){
    float dt = luaL_checknumber(L, 1);
    WorldUpdate(dt);
    return 0;
}

//endregion

static int RenderSetBufferLua(lua_State* L){
 	int width = (int) luaL_checknumber(L, 1);
	int height = (int) luaL_checknumber(L, 2);
 	dmScript::LuaHBuffer* buffer = dmScript::CheckBuffer(L, 3);
 	RenderSetBuffer(width, height, buffer);
 	return 0;
}

static int DrawScreenLua(lua_State* L){
	entityx::Entity e = checkEntity(L,1);
 	DrawScreen(e);
 	return 0;
}

// Functions exposed to Lua
static const luaL_reg Module_methods[] ={
    //render
	{"render_set_buffer", RenderSetBufferLua},
	{"draw_screen", DrawScreenLua},

	//map functions
	{"map_clear", MapClearLua},
	{"map_vertex_add", MapVertexAddLua},
	{"map_vertex_change", MapVertexChangeLua},
	{"map_sector_create", MapSectorCreateLua},
	{"map_sector_vertex_add", MapSectorVertexAddLua},
	{"map_check", MapCheckLua},


	//world
	{"world_update",WorldUpdateLua},
	{0, 0},
};

static void LuaInit(lua_State* L){
	int top = lua_gettop(L);
	// Register lua names
	luaL_register(L, MODULE_NAME, Module_methods);
	lua_pop(L, 1);
	assert(top == lua_gettop(L));

	Entity_register(L);

}

static dmExtension::Result AppInitializeMyExtension(dmExtension::AppParams* params){
	return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeMyExtension(dmExtension::Params* params){
	// Init Lua
	LuaInit(params->m_L);
	printf("Registered %s Extension\n", MODULE_NAME);
	return dmExtension::RESULT_OK;
} 

static dmExtension::Result AppFinalizeMyExtension(dmExtension::AppParams* params){
	return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeMyExtension(dmExtension::Params* params){
	return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, AppInitializeMyExtension, AppFinalizeMyExtension, InitializeMyExtension, 0, 0, FinalizeMyExtension)