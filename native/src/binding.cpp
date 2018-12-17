// myextension.cpp
// Extension lib defines
#define EXTENSION_NAME Native
#define LIB_NAME "Native"
#define MODULE_NAME "native"

#include <math.h>
#include "3d_render.h"
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

//region Player
int PlayerInitLua(lua_State* L){
    int sector = (int)luaL_checknumber(L, 1);
    float x = luaL_checknumber(L, 2), y = luaL_checknumber(L, 3);
    PlayerInit(sector,x,y);
    return 0;
}
//endregion

static char* decodeBuffer(dmBuffer::HBuffer *hBuffer, uint32_t *datasize){
	char* data = 0;
	dmBuffer::GetBytes(*hBuffer, (void**)&data, datasize);
	return data;
}

static int SetBufferLua(lua_State* L){
 	int width = (int) luaL_checknumber(L, 1);
	int height = (int) luaL_checknumber(L, 2);
 	dmScript::LuaHBuffer* buffer = dmScript::CheckBuffer(L, 3);
 	setBuffer(width, height, buffer);
 	return 0;
}

static int DrawScreenLua(lua_State* L){
 	DrawScreen();
 	return 0;
}
static int MovePlayerLua(lua_State* L){
	float x = luaL_checknumber(L, 1);
 	float y = luaL_checknumber(L, 2);
 	MovePlayer(x,y);
 	return 0;
}

static int SetAngleLua(lua_State* L){
 	float angle = luaL_checknumber(L, 1);
    SetAngle(angle);
    return 0;
}

static int SetYawLua(lua_State* L){
 	float yaw = luaL_checknumber(L, 1);
 	SetYaw(yaw);
 	return 0;
}

static int GetPlayerPosLua(lua_State* L){
 	float x=0;
 	float y = 0;
 	float z= 0;
 	GetPlayerPos(&x,&y,&z);
 	lua_pushnumber (L, x);
 	lua_pushnumber (L, y);
 	lua_pushnumber (L, z);
 	return 3;
}

// Functions exposed to Lua
static const luaL_reg Module_methods[] ={
	{"set_buffer", SetBufferLua},
	{"draw_screen", DrawScreenLua},
	{"move_player", MovePlayerLua},
	{"set_player_angle", SetAngleLua},
    {"set_player_yaw", SetYawLua},
	{"get_player_pos", GetPlayerPosLua},

	//map functions
	{"map_clear", MapClearLua},
	{"map_vertex_add", MapVertexAddLua},
	{"map_vertex_change", MapVertexChangeLua},
	{"map_sector_create", MapSectorCreateLua},
	{"map_sector_vertex_add", MapSectorVertexAddLua},
	{"map_check", MapCheckLua},

	//player
	{"player_init", PlayerInitLua},
	{0, 0},
};

static void LuaInit(lua_State* L){
	int top = lua_gettop(L);
	// Register lua names
	luaL_register(L, MODULE_NAME, Module_methods);
	lua_pop(L, 1);
	assert(top == lua_gettop(L));

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