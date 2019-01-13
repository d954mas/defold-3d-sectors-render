#include "minimap.h"
#include "buffer.h"
#include "world.h"
#define TABLE_NAME "minimap"

static Buffer minimapBuffer;

static int playerSizePixels=2, mapWidth, mapHeight;

static uint32_t PLAYER_COLOR = 0xFF880000;
static uint32_t WALL_COLOR = 0xFFFFFF00;
static uint32_t PORTAL_COLOR = 0xFF000000; 

void MinimapUpdate(entityx::Entity e){
    entityx::ComponentHandle<PositionC> posC = e.component<PositionC>();
    entityx::ComponentHandle<AngleC> angleC = e.component<AngleC>();

    MinimapClearBuffer();
    int centerX= minimapBuffer.width/2, centerY = minimapBuffer.height/2;
    //draw player at center
    DrawRect(minimapBuffer, centerX-playerSizePixels/2, centerY-playerSizePixels/2, playerSizePixels,playerSizePixels,PLAYER_COLOR);

    float bufferWScale = minimapBuffer.width/mapWidth, bufferHScale = minimapBuffer.height/mapHeight;
    for(Sector sect: WORLD.sectors){
        for(unsigned s =0; s< sect.vertex.size()-1;s++){
            //acquire the x,y coordinates of the two endpoints(vertices) of thius edge of the sector
            //transform the vertices into the player view
            XY v1 = WORLD.vertices[sect.vertex[s]], v2 = WORLD.vertices[sect.vertex[s+1]];
            float vx1 = v1.x - posC->x, vy1 = v1.y- posC->y;
            float vx2 = v2.x - posC->x, vy2 = v2.y- posC->y;
            //rotate them around player
            float pcos = angleC->anglecos, psin = angleC->anglesin;
            //x=y, y = x =)
            float tx1 = (vx1 * psin - vy1 * pcos)*bufferWScale, ty1 = (vx1 * pcos +vy1 * psin)*bufferHScale;
            float tx2 = (vx2 * psin - vy2 * pcos)*bufferWScale, ty2 = (vx2 * pcos +vy2 * psin)*bufferHScale;
            //for map zero is top left corner;
            DrawLine(minimapBuffer,centerX+tx1,centerX+ty1,centerX+tx2,centerX+ty2,WALL_COLOR,true);
        }
    }
}

void MinimapSetBuffer(int width, int height, dmScript::LuaHBuffer* luaBuffer){
 	minimapBuffer = CreateBuffer(width, height, luaBuffer);
}

void MinimapClearBuffer(){
	ClearBuffer(minimapBuffer);
}

void MinimapSetPlayerSizePixels(int size){
    playerSizePixels = size;
}

void MinimapSetMapSize(int width, int height){
    mapWidth = width;
    mapHeight = height;
}

//region BIND
static int MinimapUpdateLua(lua_State *L){
    entityx::Entity e = checkEntity(L,1);
    MinimapUpdate(e);
    return 0;
}

static int MinimapSetBufferLua(lua_State* L){
 	int width = (int) luaL_checknumber(L, 1);
	int height = (int) luaL_checknumber(L, 2);
 	dmScript::LuaHBuffer* buffer = dmScript::CheckBuffer(L, 3);
 	MinimapSetBuffer(width, height, buffer);
 	return 0;
}
static int MinimapSetPlayerSizeLua(lua_State* L){
 	int size = (int) luaL_checknumber(L, 1);
 	MinimapSetPlayerSizePixels(size);
 	return 0;
}
static int MinimapSetMapSizeLua(lua_State* L){
 	int width = (int) luaL_checknumber(L, 1);
	int height = (int) luaL_checknumber(L, 2);
 	MinimapSetMapSize(width, height);
 	return 0;
}

static const luaL_reg Meta_methods[] = {
    {"update", MinimapUpdateLua},
    {"set_buffer", MinimapSetBufferLua},
    {"set_player_size", MinimapSetPlayerSizeLua},
    {"set_map_size", MinimapSetMapSizeLua},
    {0,0}
};

void MinimapBind(lua_State *L){
    int top = lua_gettop(L);
    lua_pushstring(L, TABLE_NAME);
    lua_newtable(L);
    luaL_register(L, NULL, Meta_methods);
    lua_settable(L, -3);
    assert(top == lua_gettop(L));
}


//endregion




