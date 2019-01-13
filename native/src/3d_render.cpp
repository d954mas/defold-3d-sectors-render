//https://www.youtube.com/watch?v=HQYsFshbkYw
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dmsdk/sdk.h>
#include <math.h>
#include <vector>
#include "3d_render.h"
#include "buffer.h"
#include "entityx/entityx.h"
#include "ecs.h"

#define DLIB_LOG_DOMAIN "MapRender"
#include <dmsdk/dlib/log.h>

Buffer pixelBuffer;


//region MAP
void MapClear(){
    WORLD.reset();
}

void MapVertexAdd(float x, float y){
    WORLD.vertices.push_back((XY){x,y});
}

void MapVertexChange(int idx,float x, float y){
    if (idx <WORLD.vertices.size()){
        XY v = WORLD.vertices[idx];
        v.x = x; v.y = y;
    }else{
        dmLogError("no vertex with idx:%d", idx);
    }
}

void MapSectorCreate(float floor, float ceil){
    struct Sector s;
    s.floor = floor; s.ceil = ceil;
    WORLD.sectors.push_back(s);
}
//add to last sector in list
void MapSectorVertexAdd(int vertex,int neighbor){
    if (WORLD.sectors.size() != 0){
        Sector &s = WORLD.sectors[WORLD.sectors.size()-1];
        if (vertex >= 0 && vertex <WORLD.vertices.size()){ s.vertex.push_back(vertex);}
        else{ dmLogError("no vertex with idx:%d", vertex);return;}
        //todo check neighbors sector;
        s.neighbors.push_back(neighbor);
    }else{
        dmLogError("can't add vertex.No sectors");
    }
};

void MapCheck(){
    for(int i=0;i<WORLD.sectors.size();i++){
        Sector &s = WORLD.sectors[i];
        if(s.neighbors.size()<3){
            dmLogError("bad sector:%d", i);
            return;
        }
        for(int j=0;j<s.neighbors.size();j++){
            int n = s.neighbors[j];
            if (n!= -1 && n >= WORLD.sectors.size()){
                dmLogError("no neighbor with idx:%d", n);
            }
        }
    }
}

//draw functions
/* vline: Draw a vertical line on screen, with a different color pixel in top & bottom */
static inline void vline(int x, int y1,int y2, uint32_t top,uint32_t middle,uint32_t bottom){
    y1 = clamp(y1, 0, pixelBuffer.height-1);y2 = clamp(y2, 0, pixelBuffer.height-1);
    if(y2 == y1){DrawPixel(pixelBuffer,x,y1,top);}
    else if(y2 > y1) {
        DrawPixel(pixelBuffer,x,y1,top);
        for(int y=y1+1; y<y2; ++y){
            DrawPixel(pixelBuffer,x,y,middle);
        }
        DrawPixel(pixelBuffer,x,y2,bottom);
    }
}

void RenderClearBuffer(){
    ClearBuffer(pixelBuffer);
}

void RenderSetBuffer(int width, int height, dmScript::LuaHBuffer* luaBuffer){
	pixelBuffer = CreateBuffer(width, height, luaBuffer);
}



void DrawScreen(entityx::Entity e){
    entityx::ComponentHandle<PositionC> posC = e.component<PositionC>();
    entityx::ComponentHandle<AngleC> angleC = e.component<AngleC>();
    entityx::ComponentHandle<AngleC> HeadC = e.component<AngleC>();
    entityx::ComponentHandle<YawC> yawC = e.component<YawC>();
    entityx::ComponentHandle<EyeHeightC> eyeC = e.component<EyeHeightC>();
    entityx::ComponentHandle<SectorC> sectorC = e.component<SectorC>();
   // MovePlayer(world.player.where.x, world.player.where.y);
    //clearBuffer1(&pixelBuffer);
    const int W = pixelBuffer.width;
    const int H =  pixelBuffer.height;

    enum {MaxQueue  =32};
    struct item {int sectorno,sx1,sx2;} queue[MaxQueue ], *head = queue, *tail=queue;
    std::vector<int> ytop(W); // keep track if remaining windom(min,max) in each column
    std::vector<int> ybottom(W, H-1);
    std::vector<int> renderedsectors(WORLD.sectors.size());
    //start rendering from player sector
    *head = (struct item) {sectorC->v,0, W-1};
    if (++head == queue + MaxQueue ) head = queue;
    do{
        //pick a sector then slice from the queue to draw
        const struct item now = *tail;
        if(++tail == queue + MaxQueue) tail = queue;

        if(renderedsectors[now.sectorno] & 0x21) continue; // Odd = still rendering, 0x20 = give up try 32 times
        ++renderedsectors[now.sectorno];
        //const for pointer and const for data
        const Sector &sect = WORLD.sectors[now.sectorno];
        //render each wall of player sector that is facing towards player

        for(unsigned s =0; s< sect.vertex.size()-1;s++){
            //acquire the x,y coordinates of the two endpoints(vertices) of thius edge of the sector
            //transform the vertices into the player view
            XY v1 = WORLD.vertices[sect.vertex[s]], v2 = WORLD.vertices[sect.vertex[s+1]];
            float vx1 = v1.x - posC->x, vy1 = v1.y- posC->y;
            float vx2 = v2.x - posC->x, vy2 = v2.y- posC->y;
            //rotate them around player
            float pcos = angleC->anglecos, psin = angleC->anglesin;
            float tx1 = vx1 * psin - vy1 * pcos, tz1 = vx1 * pcos +vy1 * psin;
            float tx2 = vx2 * psin - vy2 * pcos, tz2 = vx2 * pcos +vy2 * psin;
            //is the wall in front of player
            if(tz1<=0 && tz2 <=0) continue;
               //if it partialy behind the player, clip it against player's view frustrum
            if(tz1 <= 0 || tz2<=0){
                float nearz = 1.0e-4, farz = 5, nearside = 1.0e-5, farside = 20;
                //find an intersection between the wall and approximate edges if player's view
                XY i1 = Intersect(tx1,tz1,tx2,tz2, - nearside, nearz, -farside, farz);
                XY i2 = Intersect(tx1,tz1,tx2,tz2, nearside, nearz, farside, farz);
                if(tz1 < nearz){
                    if(i1.y > 0){tx1 = i1.x;tz1 = i1.y;}
                    else{tx1 = i2.x;tz1 = i2.y;}
                }
                if(tz2 < nearz){
                    if(i1.y > 0){tx2 = i1.x;tz2 = i1.y;}
                    else{tx2 = i2.x;tz2 = i2.y;}
                }
            }
            //Perspective transformation
            float xscale1 = hfov / tz1, yscale1 = vfov/tz1; int x1 = W/2 - (int)(tx1 * xscale1);
            float xscale2 = hfov / tz2, yscale2 = vfov/tz2; int x2 = W/2 - (int)(tx2 * xscale2);
            //only render if visible
            //if right vertices is left to screen. Or lefy is right. That mean that edge not visible
            //x1 >= x2 wtf
            if(x1 >= x2 || x2 < now.sx1 || x1 > now.sx2) continue;
            //acquire the floor and ceilings height, relative to player view;
            float yceil = sect.ceil - posC->z-eyeC->v;
            float yfloor = sect.floor - posC->z-eyeC->v;


            //check the edge type, neighor=-1 means wall, other boundary between two sectors.
            int neighbor = sect.neighbors[s];

            float nyceil = 0, nyfloor = 0;
            if (neighbor>=0){
                nyceil =  WORLD.sectors[neighbor].ceil - posC->z-eyeC->v;
                nyfloor = WORLD.sectors[neighbor].floor - posC->z-eyeC->v;
            }
            #define Yaw(y,z) (y + z)//z*yawC->yaw)
            //project floor/ceiling height into screen coordinates(Y)
              int y1a  = H/2 - (int)(Yaw(yceil, tz1) * yscale1),  y1b = H/2 - (int)(Yaw(yfloor, tz1) * yscale1);
              int y2a  = H/2 - (int)(Yaw(yceil, tz2) * yscale2),  y2b = H/2 - (int)(Yaw(yfloor, tz2) * yscale2);
              /* The same for the neighboring sector */
              int ny1a = H/2 - (int)(Yaw(nyceil, tz1) * yscale1), ny1b = H/2 - (int)(Yaw(nyfloor, tz1) * yscale1);
              int ny2a = H/2 - (int)(Yaw(nyceil, tz2) * yscale2), ny2b = H/2 - (int)(Yaw(nyfloor, tz2) * yscale2);
            /*Render the wall */
            int beginx = max(x1, now.sx1), endx = min(x2, now.sx2);
            for(int x = beginx; x<=endx; ++x){
                /* Calculate the Z coordinate for this point. (Only used for lighting.) */
                int z = ((x - x1) * (tz2-tz1) / (x2-x1) + tz1) * 8;
                /* Acquire the Y coordinates for our ceiling & floor for this X coordinate. Clamp them. */
                int ya = (x - x1) * (y2a-y1a) / (x2-x1) + y1a, cya = clamp(ya, ytop[x],ybottom[x]); // top
                int yb = (x - x1) * (y2b-y1b) / (x2-x1) + y1b, cyb = clamp(yb, ytop[x],ybottom[x]); // bottom
                //render ceiling;everything above this sector's ceiling height
                vline(x,ytop[x],cya-1,0x11111100, 0x22222200, 0x11111100);
                //render floor;everything below this sector's floor height
                 vline(x,cyb+1,ybottom[x],0x0000FF00, 0x0000AA00, 0x0000FF00);

                 if(neighbor >=0){
                    //same for their floor and ceiling
                     int nya = (x-x1) * (ny2a-ny1a)/ (x2-x1) +ny1a, cnya = clamp(nya, ytop[x], ybottom[x]);//top
                     int nyb = (x-x1) * (ny2b-ny1b)/ (x2-x1) +ny1b, cnyb = clamp(nyb, ytop[x], ybottom[x]);//bottom
                     //if our ceiling in higher than their ceiling, render upper wall
                    int r1 = 0x01010100 * (255-z), r2 = 0x04000700 * (31-z/8);
                    vline(x,cya,cnya-1,0,x==x1 || x==x2 ? 0: r1,0); //between our and their ceiling
                      ytop[x] = clamp(max(cya,cnya),ytop[x], H-1);

                      //if our floor is lower than their floor, render bottom wall
                       vline(x,cnyb+1,cyb,0,x==x1 || x==x2 ? 0: r2,0); //between their and our floor
                       ybottom[x] = clamp(min(cyb,cnyb),0,ybottom[x]);

                 }else{
                    //draw wall
                    unsigned r = 0x01010100 * (255-z);
                    vline(x,cya,cyb,0,x==x1 || x==x2 ? 0 : r, 0);
                 }
            }
            //shedule the neighbors rendering with it window
            if (neighbor >=0 && endx >= beginx && (head + MaxQueue + 1 - tail)%MaxQueue){
                *head = (struct item){neighbor,beginx, endx};
                if(++head == queue + MaxQueue) head = queue;
            }
        }
    ++renderedsectors[now.sectorno];
    }while(head!= tail);
}




