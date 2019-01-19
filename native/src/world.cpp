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

void World::setHFov(float fov){
    hfov = fov;
}

void World::setVFov(float fov){
    vfov = fov;
}

World WORLD;
EcsWorld ECS;

//region BIND
static int WorldUpdateLua(lua_State* L){
    float dt = luaL_checknumber(L, 1);
    WorldUpdate(dt);
    return 0;
}

static int WorldSetHFov(lua_State* L){
    float fov = luaL_checknumber(L, 1);
    WORLD.hfov = fov;
    return 0;
}

static int WorldSetVFov(lua_State* L){
    float fov = luaL_checknumber(L, 1);
    WORLD.hfov = fov;
    return 0;
}

static const luaL_reg Meta_methods[] = {
    {"update", WorldUpdateLua},
    {"set_hfov", WorldSetHFov},
    {"set_vfov", WorldSetVFov},
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



//https://stackoverflow.com/questions/1119627/how-to-test-if-a-point-is-inside-of-a-convex-polygon-in-2d-integer-coordinates
static int get_side(vec2f a, vec2f b){
    float x = vec2f::cross(a, b);
    if (x < 0){
        return 1;
    }else if (x>0){
        return 2;
    }else{
        return 0;
    }
}

// Returns true if the point p lies inside the polygon[] with n vertices
bool IsInside(Sector&  s, vec2f& p)
{
    const std::vector<int> &vert = s.vertex;
    int n =(s.vertex.size()-1);
    if (n < 3)  return false;
    int prev_side = -1;
     for (unsigned s = 0; s < vert.size() - 1; ++s) {
        vec2f v1 = WORLD.vertices[vert[s]];
        vec2f v2 = WORLD.vertices[vert[s + 1]];
        vec2f affine_segment =  v2-v1;
        vec2f affine_point = p - v1;
        int current_side = get_side(affine_segment, affine_point);
        if (current_side == 0){
            return false;
        }
        if(prev_side == -1){
            prev_side = current_side;
        }else if(prev_side!= current_side){
            return false;
        }
      }
    return true;
}


//endregion
