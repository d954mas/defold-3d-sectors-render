#include <dmsdk/sdk.h>
#include "ecs_bind.h"
entityx::Entity checkEntity (lua_State *L, int index){
	luaL_checktype(L, index, LUA_TUSERDATA);
	entityx::Entity e = *((entityx::Entity*)  lua_touserdata(L, index));
	return e;
}

EcsWorld::EcsWorld() {
        systems.add<MovementSystem>();
        systems.add<CollisionSystem>();
        systems.configure();
}

void EcsWorld::update(entityx::TimeDelta dt) {
    systems.update<MovementSystem>(dt);
    systems.update<CollisionSystem>(dt);
}
