//https://www.youtube.com/watch?v=HQYsFshbkYw
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dmsdk/sdk.h>
#include <math.h>
#include <vector>
#include "3d_render.h"

#define DLIB_LOG_DOMAIN "MapRender"
#include <dmsdk/dlib/log.h>

/* Define window size */
/* Define various vision related constants */
#define EyeHeight  6    // Camera height from floor when standing
#define DuckHeight 2.5  // And when crouching
#define HeadMargin 1    // How much room there is above camera before the head hits the ceiling
#define KneeHeight 2    // How tall obstacles the player can simply walk over without jumping
#define hfov (0.73f*H)  // Affects the horizontal field of vision
#define vfov (.2f*H)    // Affects the vertical field of vision

// Utility functions. Because C doesn't have templates,
// we use the slightly less safe preprocessor macros to
// implement these functions that work with multiple types.
#define min(a,b)             (((a) < (b)) ? (a) : (b)) // min: Choose smaller of two scalars.
#define max(a,b)             (((a) > (b)) ? (a) : (b)) // max: Choose greater of two scalars.
#define clamp(a, mi,ma)      min(max(a,mi),ma)         // clamp: Clamp value into set range.
#define vxs(x0,y0, x1,y1)    ((x0)*(y1) - (x1)*(y0))   // vxs: Vector cross product
// Overlap:  Determine whether the two number ranges overlap.
#define Overlap(a0,a1,b0,b1) (min(a0,a1) <= max(b0,b1) && min(b0,b1) <= max(a0,a1))
// IntersectBox: Determine whether two 2D-boxes intersect.
#define IntersectBox(x0,y0, x1,y1, x2,y2, x3,y3) (Overlap(x0,x1,x2,x3) && Overlap(y0,y1,y2,y3))
// PointSide: Determine which side of a line the point is on. Return value: <0, =0 or >0.
#define PointSide(px,py, x0,y0, x1,y1) vxs((x1)-(x0), (y1)-(y0), (px)-(x0), (py)-(y0))
// Intersect: Calculate the point of intersection between two lines.
#define Intersect(x1,y1, x2,y2, x3,y3, x4,y4) ((struct xy) { \
    vxs(vxs(x1,y1, x2,y2), (x1)-(x2), vxs(x3,y3, x4,y4), (x3)-(x4)) / vxs((x1)-(x2), (y1)-(y2), (x3)-(x4), (y3)-(y4)), \
    vxs(vxs(x1,y1, x2,y2), (y1)-(y2), vxs(x3,y3, x4,y4), (y3)-(y4)) / vxs((x1)-(x2), (y1)-(y2), (x3)-(x4), (y3)-(y4)) })

struct Buffer {
	int width;
	int height;
	uint8_t* stream;
} pixelBuffer;

/* Sectors: Floor and ceiling height; list of edge vertices and neighbors */
//Sector is a room, where i can set floor and ceiling height
//Sector can be 2 types. Wall and portal. We can see throw portal
static  std::vector<struct sector> sectors;
static  std::vector<struct xy> vertices;

/*Player: location */
static struct player player;
int ground = 0, falling = 1, moving = 1;


//region MAP
void MapClear(){
     sectors.clear();
     vertices.clear();
     ground = 0, falling = 1, moving = 1;
    player = (struct player) { {0,0,0}, {0,0,0}, 0,0,0,0, 0 };
}

void MapVertexAdd(float x, float y){
    vertices.push_back((struct xy){x,y});
}

void MapVertexChange(int idx,float x, float y){
    if (idx <vertices.size()){
        struct xy v = vertices[idx];
        v.x = x; v.y = y;
    }else{
        dmLogError("no vertex with idx:%d", idx);
    }
}


void MapSectorCreate(float floor, float ceil){
    struct sector s;
    s.floor = floor; s.ceil = ceil;
    sectors.push_back(s);
}
//add to last sector in list
void MapSectorVertexAdd(int vertex,int neighbor){
    if (sectors.size() != 0){
        sector &s = sectors[sectors.size()-1];
        if (vertex >= 0 && vertex <vertices.size()){ s.vertex.push_back(vertex);}
        else{ dmLogError("no vertex with idx:%d", vertex);return;}
        //todo check neighbors sector;
        s.neighbors.push_back(neighbor);
    }else{
        dmLogError("can't add vertex.No sectors");
    }
};

void MapCheck(){
    for(int i=0;i<sectors.size();i++){
        sector &s = sectors[i];
        if(s.neighbors.size()<3){
            dmLogError("bad sector:%d", i);
            return;
        }
        for(int j=0;j<s.neighbors.size();j++){
            int n = s.neighbors[j];
            if (n!= -1 && n >= sectors.size()){
                dmLogError("no neighbor with idx:%d", n);
            }
        }
    }
}

//endregion MAP

//region PLAYER

void PlayerInit(int sector, float x, float y){
    player.where = {x,y,0};
    player.where.z = sectors[sector].floor + EyeHeight;
    player.sector = sector;
    MovePlayer(x,y);
}

//endregion



//draw functions
/* vline: Draw a vertical line on screen, with a different color pixel in top & bottom */
static inline void vline(int x, int y1,int y2, uint32_t top,uint32_t middle,uint32_t bottom)
{
    y1 = clamp(y1, 0, pixelBuffer.height-1);
    y2 = clamp(y2, 0, pixelBuffer.height-1);

    if(y2 == y1){
        uint8_t red = (top & 0xFF000000) >> 24;
        uint8_t green = (top & 0x00FF0000) >> 16;
        uint8_t blue = (top & 0x0000FF00) >> 8;
        pixelBuffer.stream[((pixelBuffer.height - y1-1)*pixelBuffer.width+x)*3] = red;
        pixelBuffer.stream[((pixelBuffer.height - y1-1)*pixelBuffer.width+x)*3+1] = green;
        pixelBuffer.stream[((pixelBuffer.height - y1-1)*pixelBuffer.width+x)*3+2] = blue;
    }

    else if(y2 > y1)
    {
        uint8_t red = (top & 0xFF000000) >> 24;
        uint8_t green = (top & 0x00FF0000) >> 16;
        uint8_t blue = (top & 0x0000FF00) >> 8;
        pixelBuffer.stream[((pixelBuffer.height - y1-1)*pixelBuffer.width+x)*3] = red;
        pixelBuffer.stream[((pixelBuffer.height - y1-1)*pixelBuffer.width+x)*3+1] = green;
        pixelBuffer.stream[((pixelBuffer.height - y1-1)*pixelBuffer.width+x)*3+2] = blue;

        red = (middle & 0xFF000000) >> 24;
        green = (middle & 0x00FF0000) >> 16;
        blue = (middle & 0x0000FF00) >> 8;
        for(int y=y1+1; y<y2; ++y){
            pixelBuffer.stream[((pixelBuffer.height - y-1)*pixelBuffer.width+x)*3] = red;
            pixelBuffer.stream[((pixelBuffer.height - y-1)*pixelBuffer.width+x)*3+1] = green;
            pixelBuffer.stream[((pixelBuffer.height - y-1)*pixelBuffer.width+x)*3+2] = blue;
        }

        red = (bottom & 0xFF000000) >> 24;
        green = (bottom & 0x00FF0000) >> 16;
        blue = (bottom & 0x0000FF00) >> 8;
        pixelBuffer.stream[((pixelBuffer.height - y2-1)*pixelBuffer.width+x)*3] = red;
        pixelBuffer.stream[((pixelBuffer.height - y2-1)*pixelBuffer.width+x)*3+1] = green;
        pixelBuffer.stream[((pixelBuffer.height - y2-1)*pixelBuffer.width+x)*3+2] = blue;
    }
}

static void clearBuffer1(struct Buffer* buffer){
	int size = buffer->width * buffer->height;
	for(int i=0; i< size*3; i+=3){
		buffer->stream[i] = 0;
		buffer->stream[i+1] = 0;
		buffer->stream[i+2] = 0;
	}
}



void MovePlayer(float x, float y){
    float eyeheight =  EyeHeight;
    const sector &sect = sectors[player.sector];
    const std::vector<int>  vert = sect.vertex;
    float px = player.where.x, py = player.where.y;
    float dx = x - player.where.x, dy =y - player.where.y;

    //vertical
     /* Vertical collision detection */
    ground = !falling;
    if(falling){
        player.velocity.z -= 0.05f; /* Add gravity */
        float nextz = player.where.z + player.velocity.z;
        if(player.velocity.z < 0 && nextz  < sectors[player.sector].floor + eyeheight) // When going down
        {
            /* Fix to ground */
            player.where.z    = sectors[player.sector].floor + eyeheight;
            player.velocity.z = 0;
            falling = 0;
            ground  = 1;
        }
        else if(player.velocity.z > 0 && nextz > sectors[player.sector].ceil) // When going up
        {
            /* Prevent jumping above ceiling */
            player.velocity.z = 0;
            falling = 1;
        }
        if(falling)
        {
            player.where.z += player.velocity.z;
            moving = 1;
        }
    }

    //horizontal


    for(unsigned s = 0; s < vert.size()-1; ++s){
        xy v =  vertices[vert[s+0]];
        xy v2 =  vertices[vert[s+1]];
        if(IntersectBox(px,py, px+dx,py+dy, v.x, v.y, v2.x, v2.y)
            && PointSide(px+dx, py+dy, v.x, v.y, v2.x, v2.y) < 0){
            /* Check where the hole is. */
            float hole_low  = sect.neighbors[s] < 0 ?  9e9 : max(sect.floor, sectors[sect.neighbors[s]].floor);
            float hole_high = sect.neighbors[s] < 0 ? -9e9 : min(sect.ceil,  sectors[sect.neighbors[s]].ceil );
            /* Check whether we're bumping into a wall. */
            if(hole_high < player.where.z+HeadMargin
            || hole_low  > player.where.z-eyeheight+KneeHeight){
                /* Bumps into a wall! Slide along the wall. */
                /* This formula is from Wikipedia article "vector projection". */
                float xd = v2.x - v.x, yd = v2.y - v.y;
                dx = xd * (dx*xd + yd*dy) / (xd*xd + yd*yd);
                dy = yd * (dx*xd + yd*dy) / (xd*xd + yd*yd);
            }
        }
    }
    falling = 1;
    for(unsigned s = 0; s < vert.size()-1; ++s){
        xy v1 =  vertices[vert[s+0]];
        xy v2 =  vertices[vert[s+1]];
        if(sect.neighbors[s] >= 0
        && IntersectBox(px,py, px+dx,py+dy, v1.x, v1.y, v2.x, v2.y)
        && PointSide(px+dx, py+dy, v1.x,v1.y, v2.x, v2.y) < 0)
        {
            player.sector = sect.neighbors[s];
            break;
        }
    }

    player.where.x = px + dx;
    player.where.y = py + dy;
}

void SetAngle(float angle){
    player.angle = angle;
    player.anglesin = sinf(player.angle);
    player.anglecos = cosf(player.angle);
}

void SetYaw(float yaw){
    player.yaw = yaw;
}

void GetPlayerPos(float *x, float *y, float *z){
    *x = player.where.x;
    *y = player.where.y;
    *z = player.where.z;
}  

void DrawScreen(){
    MovePlayer(player.where.x, player.where.y);
    //clearBuffer1(&pixelBuffer);
    const int W = pixelBuffer.width;
    const int H =  pixelBuffer.height;

    enum {MaxQueue  =32};
    struct item {int sectorno,sx1,sx2;} queue[MaxQueue ], *head = queue, *tail=queue;
    std::vector<int> ytop(W); // keep track if remaining windom(min,max) in each column
    std::vector<int> ybottom(W, H-1);
    std::vector<int> renderedsectors(sectors.size());
    //start rendering from player sector
    *head = (struct item) {player.sector,0, W-1};
    if (++head == queue + MaxQueue ) head = queue;
    do{
        //pick a sector then slice from the queue to draw
        const struct item now = *tail;
        if(++tail == queue + MaxQueue) tail = queue;

        if(renderedsectors[now.sectorno] & 0x21) continue; // Odd = still rendering, 0x20 = give up try 32 times
        ++renderedsectors[now.sectorno];
        //const for pointer and const for data
        const sector &sect = sectors[now.sectorno];
        //render each wall of player sector that is facing towards player

        for(unsigned s =0; s< sect.vertex.size()-1;s++){
            //acquire the x,y coordinates of the two endpoints(vertices) of thius edge of the sector
            //transform the vertices into the player view
            xy v1 = vertices[sect.vertex[s]], v2 = vertices[sect.vertex[s+1]];
            float vx1 = v1.x - player.where.x, vy1 = v1.y- player.where.y;
            float vx2 = v2.x - player.where.x, vy2 = v2.y- player.where.y;
            //rotate them around player
            float pcos = player.anglecos, psin = player.anglesin;
            float tx1 = vx1 * psin - vy1 * pcos, tz1 = vx1 * pcos +vy1 * psin;
            float tx2 = vx2 * psin - vy2 * pcos, tz2 = vx2 * pcos +vy2 * psin;
            //is the wall in front of player
            if(tz1<=0 && tz2 <=0) continue;
               //if it partialy behind the player, clip it against player's view frustrum
            if(tz1 <= 0 || tz2<=0){
                float nearz = 1.0e-4, farz = 5, nearside = 1.0e-5, farside = 20;
                //find an intersection between the wall and approximate edges if player's view
                struct xy i1 = Intersect(tx1,tz1,tx2,tz2, - nearside, nearz, -farside, farz);
                struct xy i2 = Intersect(tx1,tz1,tx2,tz2, nearside, nearz, farside, farz);
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
            float yceil = sect.ceil - player.where.z;
            float yfloor = sect.floor - player.where.z;


            //check the edge type, neighor=-1 means wall, other boundary between two sectors.
            int neighbor = sect.neighbors[s];

            float nyceil = 0, nyfloor = 0;
            if (neighbor>=0){
                nyceil =  sectors[neighbor].ceil - player.where.z;
                nyfloor = sectors[neighbor].floor - player.where.z;
            }
             #define Yaw(y,z) (y + z*player.yaw)
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





//buffer funs

static void initBuffer(struct Buffer* buffer, int width, int height, dmScript::LuaHBuffer* luaBuffer){
	buffer->width = width;
	buffer->height = height;
	dmBuffer::HBuffer hBuffer = luaBuffer->m_Buffer;
	uint32_t size_ignored = 0;
	uint32_t components = 0;
	uint32_t stride = 0;
	dmBuffer::Result r = dmBuffer::GetBytes(hBuffer, (void**)&buffer->stream, &size_ignored);
	/*uint8_t *** pixels = (uint8_t ***)malloc(width * sizeof(uint8_t*));
	for(int x = 0; x<width; x++){
		pixels[x] = (uint8_t **)malloc(height * sizeof(uint8_t*));
		for(int y = 0; y<height; y++){
			pixels[x][y] = &buffer->stream[(y * width + x) * 3];
		}
	}
	buffer->pixels = pixels;*/
	//dmBuffer::Result r = dmBuffer::GetStream(hBuffer, dmHashString64("rgb"),(void**)&buffer->stream , &size_ignored, &components, &stride);
}



void setBuffer(int width, int height, dmScript::LuaHBuffer* luaBuffer){
	initBuffer(&pixelBuffer, width, height, luaBuffer);
}


void clearBuffer(){
	clearBuffer1(&pixelBuffer);
}
