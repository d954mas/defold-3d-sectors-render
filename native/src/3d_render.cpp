//https://www.youtube.com/watch?v=HQYsFshbkYw
#include "3d_render.h"
#include <dmsdk/sdk.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "buffer.h"
#include "ecs.h"
#include "entityx/entityx.h"

#define DLIB_LOG_DOMAIN "MapRender"
#include <dmsdk/dlib/log.h>

#define TABLE_NAME "render"

Buffer pixelBuffer;

//draw functions
/* vline: Draw a vertical line on screen, with a different color pixel in top & bottom */
static inline void vline(int x, int y1, int y2, uint32_t top, uint32_t middle, uint32_t bottom) {
    y1 = clamp(y1, 0, pixelBuffer.height - 1);
    y2 = clamp(y2, 0, pixelBuffer.height - 1);
    if (y2 == y1) {
        DrawPixel(pixelBuffer, x, y1, top);
    } else if (y2 > y1) {
        DrawPixel(pixelBuffer, x, y1, top);
        for (int y = y1 + 1; y < y2; ++y) {
            DrawPixel(pixelBuffer, x, y, middle);
        }
        DrawPixel(pixelBuffer, x, y2, bottom);
    }
}

void RenderClearBuffer() {
    ClearBuffer(pixelBuffer);
}

void RenderSetBuffer(int width, int height, dmScript::LuaHBuffer* luaBuffer) {
    pixelBuffer = CreateBuffer(width, height, luaBuffer);
}

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

static float lerp(float v0, float v1, float t) {
		return (1 - t) * v0 + t * v1;
}

void RenderDrawScreen(entityx::Entity e) {
    WORLD.visibility.clear();
    printf("DRAW *************************\n");
    RenderClearBuffer();
    entityx::ComponentHandle<PositionC> posC = e.component<PositionC>();
    entityx::ComponentHandle<AngleC> angleC = e.component<AngleC>();
    entityx::ComponentHandle<AngleC> HeadC = e.component<AngleC>();
    entityx::ComponentHandle<YawC> yawC = e.component<YawC>();
    entityx::ComponentHandle<EyeHeightC> eyeC = e.component<EyeHeightC>();
    entityx::ComponentHandle<SectorC> sectorC = e.component<SectorC>();

    const int W = pixelBuffer.width;
    const int H = pixelBuffer.height;

    enum { MaxQueue = 32 };
    struct item {
        int sectorno, sx1, sx2;
    } queue[MaxQueue], *head = queue, *tail = queue;
    std::vector<int> ytop(W);  // keep track if remaining windom(min,max) in each column
    std::vector<int> ybottom(W, H - 1);
    std::vector<int> renderedsectors(WORLD.sectors.size());
    //start rendering from player sector
    *head = (struct item){sectorC->v, 0, W - 1};
    if (++head == queue + MaxQueue) head = queue;
    do {
        //pick a sector then slice from the queue to draw
        const struct item now = *tail;
        if (++tail == queue + MaxQueue) tail = queue;


      //    printf("sector:%d\n",now.sectorno);

        if (renderedsectors[now.sectorno] & 0x21) continue;  // Odd = still rendering, 0x20 = give up try 32 times
        ++renderedsectors[now.sectorno];
        //const for pointer and const for data
        const Sector& sect = WORLD.sectors[now.sectorno];
        std::unordered_set<int>& visible_walls = WORLD.visibility[sect.id];
        //render each wall of player sector that is facing towards player

        for (unsigned s = 0; s < sect.vertex.size() - 1; s++) {
            //acquire the x,y coordinates of the two endpoints(vertices) of thius edge of the sector
            //transform the vertices into the player view
            vec2f v1 = WORLD.vertices[sect.vertex[s]], v2 = WORLD.vertices[sect.vertex[s + 1]];
            vec2f tv1 = (v1 - posC->pos).rotate(angleC->anglecos, angleC->anglesin);
            vec2f tv2 = (v2 - posC->pos).rotate(angleC->anglecos, angleC->anglesin);
            // printf("check line:(%f,%f)(%f,%f)\n",v1.x,v1.y,v2.x,v2.y);
            //is the wall in front of player
             float nearz = 1.0e-4, farz = 5, nearside = 1.0e-5, farside = 20;
            if (tv1.y <= nearz && tv2.y <= nearz) continue;
            // printf("infront\n");
            // printf("tv1.y:%f tv2.y:%f\n",tv1.y,tv2.y);
            //if it partialy behind the player, clip it against player's view frustrum
            bool draw_part = false;
            if (tv1.y <= nearz || tv2.y <= nearz) {
                float r0x = tv1.y, r0y = tv1.x, r1x = tv2.y, r1y= tv2.x;


               // float r0x = tv1.x, r0y = tv1.y, r1x = tv2.x, r1y= tv2.y;
                // Distance between the vertices and the near z
                float da = r0x  - nearz;
                float db = r1x  - nearz;
                bool left = false;
                if (sgn(da) != sgn(db)) {
                    float s = da / (db - da);

                	if (r0x <= 0) {
                        r0x = r1x - (1 + s) * (r1x - r0x);
                        r0y = r1y - (1+ s) * (r1y - r0y);
                        left = true;
                    } else {
                         r1x = r1x - (1+s) * (r1x - r0x);
                         r1y = r1y - (1+s) * (r1y - r0y);
                    }
                } else {
                    printf("ERROR:\n");
                }
              //  tv1.x = r0x;tv1.y = r0y;tv2.x = r1x;tv2.y = r1y;
                vec2f tv1a = vec2f(tv1);
                vec2f tv2a = vec2f(tv2);



            // if (x1 > x2) {
               //int i = x1; x1 = x2; x2 = i;
           //  }

      
               // if(tv1.x < tv2.x){
                  //  float i = tv1.x;tv1.x = tv2.x;tv2.x = i;
               // }
           // }
               // continue;
              /*  if (fabs(r1y - r0y) > fabs(r1x - r0x)) {
                    u0 = (int) ((r0y - or0y) / (or1y - or0y) * segLength);
                    u1 = (int) ((r1y - or0y) / (or1y - or0y) * segLength);
                } else {
                    u0 = (int) ((r0x - or0x) / (or1x - or0x) * segLength);
                    u1 = (int) ((r1x - or0x) / (or1x - or0x) * segLength);
                }*/


                //  printf("partialy\n");

                //find an intersection between the wall and approximate edges if player's view
                vec2f i1 = vec2f(0,0);
                vec2f i2 = vec2f(0,0);
                //  if (i1p != NULL && i2p != NULL) {
                // Intersect2(tv1, tv2, vec2f(-nearside, nearz), vec2f(-farside, farz),i1);;
                //  Intersect2(tv1, tv2, vec2f(nearside, nearz), vec2f(farside, farz),i2);;

                if (Intersect2(tv1a, tv2a, vec2f(-nearside, nearz), vec2f(-farside, farz), i1)
                && Intersect2(tv1a, tv2a, vec2f(nearside, nearz), vec2f(farside, farz), i2)) {
                    if (tv1a.y < 0) {
                        if (i1.y > 0) {
                            tv1a = vec2f(i1);
                        } else {
                            tv1a = vec2f(i2);
                        }
                    }
                    if (tv2a.y < 0) {
                        if (i1.y > 0) {
                            tv2a = vec2f(i1);
                        } else {
                            tv2a = vec2f(i2);
                        }
                    }

                    
                }else{
                    printf("no interseck\n");
                }    

                float xscale1 = hfovm / r0x, yscale1 = vfovm / r0x;
                int x1 = W / 2 - (int)(r0y * xscale1);
                float xscale2 = hfovm / r1x, yscale2 = vfovm / r1x;
                int x2 = W / 2 - (int)(r1y * xscale2);
                 if (!(x1 >= x2 || x2 < now.sx1 || x1 > now.sx2)){
                     // printf("***************************\n");
                     // printf("hfov:%f\n", hfovm);
                     // printf("tv1:(%f;%f) tv2:(%f,%f)\n",tv1.x,tv1.y,tv2.x,tv2.y);
                      xscale1 = hfovm / tv1.y, yscale1 = vfovm / tv1.y;
                      x1 = W / 2 - (int)(tv1.x * xscale1);
                      xscale2 = hfovm / tv2.y, yscale2 = vfovm / tv2.y;
                      x2 = W / 2 - (int)(tv2.x * xscale2);
                     // printf("x1:(%d) x2:(%d)\n",x1,x2);

                      //new way
                     //  printf("tv1:(%f;%f) tv2:(%f,%f)\n",r0y,r0x,r1y,r1x);
                      xscale1 = hfovm / r0x, yscale1 = vfovm / r0x;
                      x1 = W / 2 - (int)(r0y * xscale1);
                      xscale2 = hfovm / r1x, yscale2 = vfovm / r1x;
                      x2 = W / 2 - (int)(r1y * xscale2);
                     //  printf("x1:(%d) x2:(%d)\n",x1,x2);

                       int beginx = max(x1, now.sx1), endx = min(x2, now.sx2);
                       if(left && beginx!=x1){


                       }
                       if(not left && endx!=x2){

                       }

                       //old
                     //  printf("tv1:(%f;%f) tv2:(%f,%f)\n",tv1a.x,tv1a.y,tv2a.x,tv2a.y);
                       xscale1 = hfovm / tv1a.y, yscale1 = vfovm / tv1a.y;
                       x1 = W / 2 - (int)(tv1a.x * xscale1);
                       xscale2 = hfovm / tv2a.y, yscale2 = vfovm / tv2a.y;
                       x2 = W / 2 - (int)(tv2a.x * xscale2);
                    //   printf("x1:(%d) x2:(%d)\n",x1,x2);
                 };
                   tv1.x = tv1a.x;tv1.y = tv1a.y;tv2.x = tv2a.x;tv2.y = tv2a.y;
                    // tv1.x = r0y;tv1.y = r0x;tv2.x = r1y;tv2.y = r1x;
                 }


            //Perspective transformation
            float xscale1 = hfovm / tv1.y, yscale1 = vfovm / tv1.y;
            int x1 = W / 2 - (int)(tv1.x * xscale1);
            float xscale2 = hfovm / tv2.y, yscale2 = vfovm / tv2.y;
            int x2 = W / 2 - (int)(tv2.x * xscale2);
           //  printf("x1:(%d) x2:(%d)\n",x1,x2);

            //only render if visible
            //if right vertices is left to screen. Or lefy is right. That mean that edge not visible
            //x1 >= x2 wtf
            // printf("sx1:%d sx2:%d\n",now.sx1,now.sx2);
            // printf("x1:%d x2:%d\n",x1,x2);
            if (x1 >= x2 || x2 < now.sx1 || x1 > now.sx2) continue;

            visible_walls.insert(s);
            int size = visible_walls.size();
           // printf("size:%d",size);
            //printf("size:%d\n",size);
            // printf("visible\n");
            //acquire the floor and ceilings height, relative to player view;
            float yceil = sect.ceil - posC->z - eyeC->v;
            float yfloor = sect.floor - posC->z - eyeC->v;

            //check the edge type, neighor=-1 means wall, other boundary between two sectors.
            int neighbor = sect.neighbors[s];

            float nyceil = 0, nyfloor = 0;
            if (neighbor >= 0) {
                nyceil = WORLD.sectors[neighbor].ceil - posC->z - eyeC->v;
                nyfloor = WORLD.sectors[neighbor].floor - posC->z - eyeC->v;
            }
#define Yaw(y, z) (y + z)  //z*yawC->yaw) \
                           //project floor/ceiling height into screen coordinates(Y)
            int y1a = H / 2 - (int)(Yaw(yceil, tv1.y) * yscale1), y1b = H / 2 - (int)(Yaw(yfloor, tv1.y) * yscale1);
            int y2a = H / 2 - (int)(Yaw(yceil, tv2.y) * yscale2), y2b = H / 2 - (int)(Yaw(yfloor, tv2.y) * yscale2);
            /* The same for the neighboring sector */
            int ny1a = H / 2 - (int)(Yaw(nyceil, tv1.y) * yscale1), ny1b = H / 2 - (int)(Yaw(nyfloor, tv1.y) * yscale1);
            int ny2a = H / 2 - (int)(Yaw(nyceil, tv2.y) * yscale2), ny2b = H / 2 - (int)(Yaw(nyfloor, tv2.y) * yscale2);
            /*Render the wall */
            int beginx = max(x1, now.sx1), endx = min(x2, now.sx2);
            for (int x = beginx; x <= endx; ++x) {
                float lerpFactor = (x - x) / (x1 - x);
                /* Calculate the Z coordinate for this point. (Only used for lighting.) */
                int z = ((x - x1) * (tv2.y - tv1.y) / (x2 - x1) + tv1.y) * 8;
                /* Acquire the Y coordinates for our ceiling & floor for this X coordinate. Clamp them. */
                int ya = (x - x1) * (y2a - y1a) / (x2 - x1) + y1a, cya = clamp(ya, ytop[x], ybottom[x]);  // top
                int yb = (x - x1) * (y2b - y1b) / (x2 - x1) + y1b, cyb = clamp(yb, ytop[x], ybottom[x]);  // bottom



                //render ceiling;everything above this sector's ceiling height
                vline(x, ytop[x], cya - 1, 0x11111100, 0x22222200, 0x11111100);
                //render floor;everything below this sector's floor height
                vline(x, cyb + 1, ybottom[x], 0x0000FF00, 0x0000AA00, 0x0000FF00);

                if (neighbor >= 0) {
                    //same for their floor and ceiling
                    int nya = (x - x1) * (ny2a - ny1a) / (x2 - x1) + ny1a, cnya = clamp(nya, ytop[x], ybottom[x]);  //top
                    int nyb = (x - x1) * (ny2b - ny1b) / (x2 - x1) + ny1b, cnyb = clamp(nyb, ytop[x], ybottom[x]);  //bottom
                                                                                                                    //if our ceiling in higher than their ceiling, render upper wall
                    int r1 = 0x01010100 * (255 - z), r2 = 0x04000700 * (31 - z / 8);
                    vline(x, cya, cnya - 1, 0, x == x1 || x == x2 ? 0 : r1, 0);  //between our and their ceiling
                    ytop[x] = clamp(max(cya, cnya), ytop[x], H - 1);

                    //if our floor is lower than their floor, render bottom wall
                    vline(x, cnyb + 1, cyb, 0, x == x1 || x == x2 ? 0 : r2, 0);  //between their and our floor
                    ybottom[x] = clamp(min(cyb, cnyb), 0, ybottom[x]);

                } else {
                    //draw wall
                    unsigned r = 0x01010100 * (255 - z);
                    vline(x, cya, cyb, 0, x == x1 || x == x2 ? 0 : r, 0);
                }
            }
            //shedule the neighbors rendering with it window
            if (neighbor >= 0 && endx >= beginx && (head + MaxQueue + 1 - tail) % MaxQueue) {
                *head = (struct item){neighbor, beginx, endx};
                if (++head == queue + MaxQueue) head = queue;
            }
        }
        ++renderedsectors[now.sectorno];
    } while (head != tail);
}

//region BIND
static int RenderSetBufferLua(lua_State* L) {
    int width = (int)luaL_checknumber(L, 1);
    int height = (int)luaL_checknumber(L, 2);
    dmScript::LuaHBuffer* buffer = dmScript::CheckBuffer(L, 3);
    RenderSetBuffer(width, height, buffer);
    return 0;
}

static int RenderDrawScreenLua(lua_State* L) {
    entityx::Entity e = checkEntity(L, 1);
    RenderDrawScreen(e);
    return 0;
}

static int RenderGetVisible(lua_State* L) {
    lua_newtable(L);
    // populate the table with values which we want to return

    for(auto const &pair : WORLD.visibility){
    std::unordered_set<int> walls= pair.second;
    int size = walls.size();
        if(walls.size() > 0){
            int i = 0;
            lua_newtable(L);
            for(auto wall : walls){
                i++;
                lua_pushnumber(L, wall);
                lua_rawseti(L, -2, i);
            }
            lua_rawseti(L, -2, pair.first);
               // lua_pushnumber(L, it.second);
        }
       // lua_setField(L, -2, field);
    }
    return 1;
}

static const luaL_reg Meta_methods[] = {
    {"set_buffer", RenderSetBufferLua},
    {"draw_screen", RenderDrawScreenLua},
    {"get_visible", RenderGetVisible},
    {0, 0}};

void RenderBind(lua_State* L) {
    int top = lua_gettop(L);
    lua_pushstring(L, TABLE_NAME);
    lua_newtable(L);
    luaL_register(L, NULL, Meta_methods);
    lua_settable(L, -3);
    assert(top == lua_gettop(L));
}

//endregion
