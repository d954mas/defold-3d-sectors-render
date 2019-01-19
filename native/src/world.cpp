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

bool Intersect2(vec2f a1,vec2f a2,vec2f b1,vec2f b2, vec2f& result){
//#define Intersect(x1,y1, x2,y2, x3,y3, x4,y4) vec2f( \
  //    vxs(vxs1, (x1)-(x2), vxs(x3,y3, x4,y4), (x3)-(x4)) / vxs((x1)-(x2), (y1)-(y2), (x3)-(x4), (y3)-(y4)), \
  //    vxs(vxs1, (y1)-(y2), vxs(x3,y3, x4,y4), (y3)-(y4)) / vxs((x1)-(x2), (y1)-(y2), (x3)-(x4), (y3)-(y4)))

    float vxs3 = vec2f::cross(a1-a2,b1-b2);

    if(vxs3 == 0){
        return false;
    }
    

    float vxs1 = vec2f::cross(a1,a2);
    float vxs2 = vec2f::cross(b1,b2);

    float x = vec2f::cross(vec2f(vxs1, a1.x-a2.x),vec2f(vxs2,b1.x-b2.x)) / vxs3 ;
    float y = vec2f::cross(vec2f(vxs1, a1.y-a2.y),vec2f(vxs2,b1.y-b2.y)) / vxs3 ;
    result.x = x;
    result.y = y;
    return true;
}

bool Intersect4(vec2f a1,vec2f a2,vec2f b1,vec2f b2,vec2f& result){
    // equations-to-code conversion
    float x1 = a1.x, x2 = a2.x, x3 = b1.x, x4 = b2.x;
    float y1 = a1.y, y2 = a2.y, y3 = b1.y, y4 = b2.y;

    float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    // If d is zero, there is no intersection
    if (d == 0) return false;

    // Get the x and y
    float pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
    float x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
    float y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;

    // Check if the x and y coordinates are within both lines
    if ( x < min(x1, x2) || x > max(x1, x2) ||
    x < min(x3, x4) || x > max(x3, x4) ) return false;
    if ( y < min(y1, y2) || y > max(y1, y2) ||
    y < min(y3, y4) || y > max(y3, y4) ) return false;
    Intersect2(a1,a1,b1,b2,result);
    return true;
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
