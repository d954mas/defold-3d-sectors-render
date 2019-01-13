#include "ecs.h"
#include <dmsdk/sdk.h>
#define ENTITY "Entity"
#define ENTITY_VAR "EntityVar"

entityx::Entity checkEntity (lua_State *L, int index);

static void pushEntity (lua_State *L, entityx::Entity e){
	entityx::Entity *l_e = (entityx::Entity *)lua_newuserdata(L, sizeof(entityx::Entity));
	*l_e = e;
	luaL_getmetatable(L, ENTITY_VAR);
	lua_setmetatable(L, -2);
}

//region ENTITIES
static int Entities_new_unit (lua_State *L){
    entityx::Entity e = ECS.entities.create();
    e.assign<PositionC>(0,0,0);
    e.assign<VelocityC>(0,0,0);
    e.assign<MovementSpeedC>();
    e.assign<HandleCollisionC>();
    e.assign<HandleGravityC>();
    e.assign<SectorC>(0);
    e.assign<AngleC>(0);
    e.assign<YawC>(0);
    e.assign<HeadMarginC>(1);
    e.assign<EyeHeightC>(6);
    e.assign<KneeHeightC>(2);
 	pushEntity(L, e);
 	return 1;
}
static int Entities_get_size (lua_State *L){
	lua_pushnumber(L, ECS.entities.size());
	return 1;
}

static const luaL_reg Entities_methods[] = {
	{"new_unit",           Entities_new_unit},
	{"get_size",           Entities_get_size},
	{0,0}
};

static const luaL_reg Entities_meta[] = {{0, 0}};


//region ENTITY
static int Entity_get_position(lua_State *L){
    entityx::Entity e = checkEntity(L, 1);
    entityx::ComponentHandle<PositionC> pos = e.component<PositionC>();
    lua_pushnumber(L, pos->x);
    lua_pushnumber(L, pos->y);
    lua_pushnumber(L, pos->z);
    return 3;
}
static int Entity_set_position(lua_State *L){
    entityx::Entity e = checkEntity(L, 1);
    entityx::ComponentHandle<PositionC> pos = e.component<PositionC>();
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    float z = luaL_checknumber(L, 4);
    pos->x = x;
    pos->y = y;
    pos->z = z;
    return 0;
}

static int Entity_get_velocity(lua_State *L){
    entityx::Entity e = checkEntity(L, 1);
    entityx::ComponentHandle<VelocityC> vel = e.component<VelocityC>();
    lua_pushnumber(L, vel->x);
    lua_pushnumber(L, vel->y);
    lua_pushnumber(L, vel->z);
    return 3;
}
static int Entity_set_velocity(lua_State *L){
    entityx::Entity e = checkEntity(L, 1);
    entityx::ComponentHandle<VelocityC> pos = e.component<VelocityC>();
    if(lua_isnoneornil(L,2)==0){
        pos->x = luaL_checknumber(L, 2);
    }
     if(lua_isnoneornil(L,3)==0){
        pos->y = luaL_checknumber(L, 3);
     }
     if(lua_isnoneornil(L,4)==0){
        pos->z = luaL_checknumber(L, 4);
     }
    return 0;
}

static int Entity_get_movement_speed(lua_State *L){
    entityx::Entity e = checkEntity(L, 1);
    entityx::ComponentHandle<MovementSpeedC> v = e.component<MovementSpeedC>();
    lua_pushnumber(L, v->v);
    return 1;
}
static int Entity_set_movement_speed(lua_State *L){
    entityx::Entity e = checkEntity(L, 1);
    entityx::ComponentHandle<MovementSpeedC> v = e.component<MovementSpeedC>();
    float v1 = luaL_checknumber(L, 2);
    v->v = v1;
    return 0;
}

static int Entity_get_sector(lua_State *L){
    entityx::Entity e = checkEntity(L, 1);
    entityx::ComponentHandle<SectorC> s = e.component<SectorC>();
    lua_pushnumber(L, s->v);
    return 1;
}
static int Entity_set_sector(lua_State *L){
    entityx::Entity e = checkEntity(L, 1);
    entityx::ComponentHandle<SectorC> v = e.component<SectorC>();
    int v1 = (int)luaL_checknumber(L, 2);
 //   v->v = v1;
    return 0;
}

static int Entity_get_angle(lua_State *L){
    entityx::Entity e = checkEntity(L, 1);
    entityx::ComponentHandle<AngleC> v = e.component<AngleC>();
    lua_pushnumber(L, v->angle);
    return 1;
}
static int Entity_set_angle(lua_State *L){
    entityx::Entity e = checkEntity(L, 1);
    entityx::ComponentHandle<AngleC> v = e.component<AngleC>();
    float v1 = luaL_checknumber(L, 2);
    v->setAngle(v1);
    return 0;
}

static int Entity_get_head_margin(lua_State *L){
    entityx::Entity e = checkEntity(L, 1);
    entityx::ComponentHandle<HeadMarginC> v = e.component<HeadMarginC>();
    lua_pushnumber(L, v->v);
    return 1;
}
static int Entity_set_head_margin(lua_State *L){
    entityx::Entity e = checkEntity(L, 1);
    entityx::ComponentHandle<HeadMarginC> v = e.component<HeadMarginC>();
    float v1 = luaL_checknumber(L, 2);
    v->v = v1;
    return 0;
}

static int Entity_get_eye_height(lua_State *L){
    entityx::Entity e = checkEntity(L, 1);
    entityx::ComponentHandle<EyeHeightC> v = e.component<EyeHeightC>();
    lua_pushnumber(L, v->v);
    return 1;
}
static int Entity_set_eye_height(lua_State *L){
    entityx::Entity e = checkEntity(L, 1);
    entityx::ComponentHandle<EyeHeightC> v = e.component<EyeHeightC>();
    float v1 = luaL_checknumber(L, 2);
    v->v = v1;
    return 0;
}

static int Entity_get_knee_height(lua_State *L){
     entityx::Entity e = checkEntity(L, 1);
     entityx::ComponentHandle<KneeHeightC> v = e.component<KneeHeightC>();
     lua_pushnumber(L, v->v);
     return 1;
}

static int Entity_is_falling(lua_State *L){
    entityx::Entity e = checkEntity(L, 1);
    entityx::ComponentHandle<HandleGravityC> v = e.component<HandleGravityC>();
    lua_pushboolean(L, v->falling);
    return 1;
}

static int Entity_set_knee_height(lua_State *L){
    entityx::Entity e = checkEntity(L, 1);
    entityx::ComponentHandle<KneeHeightC> v = e.component<KneeHeightC>();
    float v1 = luaL_checknumber(L, 2);
    v->v = v1;
    return 0;
}

static int Entity_destroy (lua_State *L){
	entityx::Entity e = checkEntity(L, 1);
	if (e.valid()){
        e.destroy();
    }
	return 0;
}


static int Entity_gc (lua_State *L){
	entityx::Entity e = checkEntity(L, 1);
	if (e.valid()){
        dmLogError("entity gc but was not destroyed\n");
        e.destroy();
	}
	return 0;
}

static const luaL_reg Entity_methods[] = {
    {"get_position", Entity_get_position},
    {"set_position", Entity_set_position},
    {"get_velocity", Entity_get_velocity},
    {"set_velocity", Entity_set_velocity},
    {"set_movement_speed", Entity_set_movement_speed},
    {"get_movement_speed", Entity_get_movement_speed},
    {"get_sector", Entity_get_sector},
    {"set_sector", Entity_set_sector},
    {"get_angle", Entity_get_angle},
    {"set_angle", Entity_set_angle},
    {"get_head_margin", Entity_get_head_margin},
    {"set_head_margin", Entity_set_head_margin},
    {"get_eye_height", Entity_get_eye_height},
    {"set_eye_height", Entity_set_eye_height},
    {"get_knee_height", Entity_get_knee_height},
    {"set_knee_height", Entity_set_knee_height},
    {"is_falling", Entity_is_falling},
    {"destroy", Entity_destroy},
	{0,0}
};

static const luaL_reg Entity_meta[] = {
	{"__gc",       Entity_gc},
	{0, 0}
};

static int Entity_register (lua_State *L)
{
	luaL_register(L, ENTITY, Entities_methods); // create methods table,
	luaL_newmetatable(L, ENTITY);
	luaL_register(L, NULL, Entities_meta);
	lua_pushliteral(L, "__index");
	lua_pushvalue(L, -3);
	lua_rawset(L, -3);
	lua_pushliteral(L, "__metatable");
	lua_pushvalue(L, -3);
	lua_rawset(L, -3);
	lua_pop(L, 1);
	lua_pop(L, 1);

	luaL_register(L, ENTITY_VAR, Entity_methods); // create methods table,
    luaL_newmetatable(L, ENTITY_VAR);
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

