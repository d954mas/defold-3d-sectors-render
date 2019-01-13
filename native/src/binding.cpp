// myextension.cpp
// Extension lib defines
#define EXTENSION_NAME Native
#define LIB_NAME "Native"
#define MODULE_NAME "native"

#include <math.h>
#include "3d_render.h"
#include "ecs_bind.h"
#include "minimap.h"
#include "map.h"
#include <dmsdk/sdk.h>


static const luaL_reg Module_methods[] ={
	{0, 0},
};

static void LuaInit(lua_State* L){
	int top = lua_gettop(L);
	// Register lua names
	luaL_register(L, MODULE_NAME, Module_methods);
	MinimapBind(L);
	MapBind(L);
	RenderBind(L);
	WorldBind(L);
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