#include "ecs.h"
#define ENTITY "Entity"

static entityx::Entity* checkEntity (lua_State *L, int index){
	luaL_checktype(L, index, LUA_TUSERDATA);
	entityx::Entity** e = (entityx::Entity**)  lua_touserdata(L, index);
	return *e;
}

static void pushEntity (lua_State *L, entityx::Entity* e){
	entityx::Entity **l_e = (entityx::Entity **)lua_newuserdata(L, sizeof(entityx::Entity*));
	*l_e = e;
	luaL_getmetatable(L, ENTITY);
	lua_setmetatable(L, -2);
}

static int Entity_new_unit (lua_State *L){
	entityx::Entity e = world.ecs.entities.create();
	pushEntity(L, &e);
	return 1;
}


static int Entity_gc (lua_State *L){
	entityx::Entity *e = checkEntity(L, 1);
	//add check here that e is already removed from world
	//if not log warning
	return 0;
}



static const luaL_reg Entity_methods[] = {
    {"new_unit",           Entity_new_unit},
	{0,0}
};

static const luaL_reg Entity_meta[] = {
	{"__gc",       Entity_gc},
	{0, 0}
};

static int Entity_register (lua_State *L)
{
	luaL_register(L, ENTITY, Entity_methods); // create methods table,
	luaL_newmetatable(L, ENTITY);
	luaL_register(L, NULL, Entity_meta);
	lua_pushliteral(L, "__index");
	lua_pushvalue(L, -3);
	lua_rawset(L, -3);
	lua_pushliteral(L, "__metatable");
	lua_pushvalue(L, -3);
	lua_rawset(L, -3);
	lua_pop(L, 1);
	lua_pop(L, 1);
	return 0;
}

