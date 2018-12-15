//https://www.youtube.com/watch?v=HQYsFshbkYw
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dmsdk/sdk.h>
#include <math.h>
#include <vector>



/* Define window size */
/* Define various vision related constants */
#define EyeHeight  6    // Camera height from floor when standing
#define DuckHeight 2.5  // And when crouching
#define HeadMargin 1    // How much room there is above camera before the head hits the ceiling
#define KneeHeight 2    // How tall obstacles the player can simply walk over without jumping
#define hfov (0.73f*H)  // Affects the horizontal field of vision
#define vfov (.2f*H)    // Affects the vertical field of vision

struct xy {float x,y;};
struct Buffer {
	int width;
	int height;
	uint8_t* stream;
} pixelBuffer;

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
        pixelBuffer.stream[((pixelBuffer.height - y2)*pixelBuffer.width+x)*3] = red;
        pixelBuffer.stream[((pixelBuffer.height - y2)*pixelBuffer.width+x)*3+1] = green;
        pixelBuffer.stream[((pixelBuffer.height - y2)*pixelBuffer.width+x)*3+2] = blue;
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


/* Sectors: Floor and ceiling height; list of edge vertices and neighbors */
//Sector is a room, where i can set floor and ceiling height
//Sector can be 2 types. Wall and portal. We can see throw portal
static struct sector
{
    float floor, ceil;
    struct xy *vertex; //vertex have x,y coords
    signed char *neighbors; //Each edge may have a corresponding neighboring sector
    unsigned int npoints; //vertexes number
} *sectors = NULL;
static unsigned NumSectors = 0;

/*Player: location */
static struct player{
    struct xyz {float x,y,z;} where, velocity;
    float angle, anglesin, anglecos, yaw;//looking towards(sin() and cos() thereof) yaw is rotation of player?
    unsigned sector; //current sector
} player;

void MovePlayer(float x, float y){
    float px = player.where.x, py = player.where.y;
    float dx,dy = x - player.where.x; y - player.where.y;
    /* Check if this movement crosses one of this sector's edges
     * that have a neighboring sector on the other side.
     * Because the edge vertices of each sector are defined in
     * clockwise order, PointSide will always return -1 for a point
     * that is outside the sector and 0 or 1 for a point that is inside.
     */
    const struct sector* const sect = &sectors[player.sector];
    const struct xy* const vert = sect->vertex;
    for(unsigned s = 0; s < sect->npoints; ++s)
        if(sect->neighbors[s] >= 0
        && IntersectBox(px,py, px+dx,py+dy, vert[s+0].x, vert[s+0].y, vert[s+1].x, vert[s+1].y)
        && PointSide(px+dx, py+dy, vert[s+0].x, vert[s+0].y, vert[s+1].x, vert[s+1].y) < 0)
        {
            player.sector = sect->neighbors[s];
            break;
        }

    player.where.x = x;
    player.where.y = y;
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

void UnloadLevel()
{
    for(unsigned a=0; a<NumSectors; ++a) free(sectors[a].vertex);
    for(unsigned a=0; a<NumSectors; ++a) free(sectors[a].neighbors);
    free(sectors);
    sectors    = NULL;
    NumSectors = 0;
}

void LoadLevel(char *data, uint32_t datasize){
    UnloadLevel();
    printf("load level");
    char *pch;
    char word[256], *ptr;
    struct xy *vert = NULL, v;
    int n, m, NumVertices = 0;
    pch = strtok (data,"\n");
     while (pch != NULL){
        ptr = pch;
        sscanf(ptr, "%32s%n", word, &n);
        switch(sscanf(ptr, "%32s%n", word, &n) == 1 ? word[0] : '\0'){
            case 'v': {// vertex
                for(sscanf(ptr += n, "%f%n", &v.y, &n); sscanf(ptr += n, "%f%n", &v.x, &n) == 1; )
                    { vert = (xy *)realloc(vert, ++NumVertices * sizeof(*vert)); vert[NumVertices-1] = v; }
                    break;}
            case 's': {// sector
                sectors = (sector *)realloc(sectors, ++NumSectors * sizeof(*sectors));
                 struct sector* sect = &sectors[NumSectors-1];
                 int* num = NULL;
                sscanf(ptr += n, "%f%f%n", &sect->floor,&sect->ceil, &n);
                for(m=0; sscanf(ptr += n, "%32s%n", word, &n) == 1 && word[0] != '#'; ){
                    num = (int *)realloc(num, ++m * sizeof(*num)); num[m-1] = atoi(word); }
                sect->npoints   = m /= 2;
                sect->neighbors = (signed char *)malloc( (m  ) * sizeof(*sect->neighbors) );
                sect->vertex    = (xy *) malloc( (m+1) * sizeof(*sect->vertex)    );
                for(n=0; n<m; ++n) sect->neighbors[n] = num[m + n];
                for(n=0; n<m; ++n) sect->vertex[n+1]  = vert[num[n]]; // TODO: Range checking
                sect->vertex[0] = sect->vertex[m]; //first and last vertex in array same
                free(num);
                break;}
            case 'p':{ // player
                float angle;
                sscanf(ptr += n, "%f %f %f %d", &v.x, &v.y, &angle,&n);
                player = (struct player) { {v.x, v.y, 0}, {0,0,0}, angle,0,0,0, n }; // TODO: Range checking
                player.where.z = sectors[player.sector].floor + EyeHeight;}
        }
        pch = strtok (NULL, "\n");
      }


    free(vert);
    printf("\nload done\n");
    MovePlayer(player.where.x, player.where.y);
    SetAngle(0);
}





void DrawScreen(){
    clearBuffer1(&pixelBuffer);
    const int W = pixelBuffer.width;
    const int H =  pixelBuffer.height;
    struct {int sectorno,sx1,sx2;} now = {player.sector,0, W-1};

    std::vector<int> ytop(W); // keep track if remaining windom(min,max) in each column
    std::vector<int> ybottom(W);
    for(unsigned x=0;x<W;x++) ybottom[x] = H-1;

    //const for pointer and const for data
    const struct sector* const sect = &sectors[now.sectorno];
    //render each wall of player sector that is facing towards player
    for(unsigned s =0; s< sect->npoints;s++){
        //acquire the x,y coordinates of the two endpoints(vertices) of thius edge of the sector
        //transform the vertices into the player view
        float vx1 = sect->vertex[s].x - player.where.x, vy1 = sect->vertex[s].y- player.where.y;
        float vx2 = sect->vertex[s+1].x - player.where.x, vy2 = sect->vertex[s+1].y- player.where.y;
        //rotate them around player
        printf("angle:%f", player.angle);
        float pcos = player.anglecos, psin = player.anglesin;
        float tx1 = vx1 * psin - vy1 * pcos, tz1 = vx1 * pcos +vy1 * psin;
        float tx2 = vx2 * psin - vy2 * pcos, tz2 = vx2 * pcos +vy2 * psin;
        //is the wall in front of player
        if(tz1<=0 && tz2 <=0) continue;
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

        //if it partialy behind the player, clip it against player's view frustrum

        //Perspective transformation
        float xscale1 = hfov / tz1, yscale1 = vfov/tz1; int x1 = W/2 - (int)(tx1 * xscale1);
        float xscale2 = hfov / tz2, yscale2 = vfov/tz2; int x2 = W/2 - (int)(tx2 * xscale2);
        //only render if visible
        //if right vertices is left to screen. Or lefy is right. That mean that edge not visible
        //x1 >= x2 wtf
        if(x1 >= x2 || x2 < now.sx1 || x1 > now.sx2) continue;
        //acquire the floor and ceilings height, relative to player view;
        float yceil = sect->ceil - player.where.z;
        float yfloor = sect->floor - player.where.z;

        //check the edge type, neighor=-1 means wall, other boundary between two sectors.
        int neighbor = sect->neighbors[s];
        //project floor/ceiling height into screen coordinates(Y)
        int y1a = H/2 - (int)(yceil * yscale1), y1b = H/2 - (int)(yfloor * yscale1);
        int y2a = H/2 - (int)(yceil * yscale2), y2b = H/2 - (int)(yfloor * yscale2);

        /*Render the wall */
        int beginx = max(x1, now.sx1), endx = min(x2, now.sx2);
        for(int x = beginx; x<=endx; x++){
            //acquire the Y coord for floor/ceiling for this x coordinate. Clam them
            int ya = (x-x1) * (y2a-y1a)/ (x2-x1) +y1a, cya = clamp(ya, ytop[x], ybottom[x]);//top
            int yb = (x-x1) * (y2b-y1b)/ (x2-x1) +y1b, cyb = clamp(yb, ytop[x], ybottom[x]);//top
            //render ceiling;everything above this sector's ceiling height
            vline(x,ytop[x],cya-1,0x11111100, 0x22222200, 0x11111100);
            //render floor;everything below this sector's floor height
             vline(x,cyb+1,ybottom[x],0x0000FF00, 0x0000AA00, 0x0000FF00);

             if(neighbor >0){
                //placeholder
                  vline(x,cya,cyb,0x00AA0000,0x00AA0000,0x00AA0000);
             }else{
                //draw wall
                vline(x,cya,cyb,0,x==x1 || x==x2 ? 0x00000000 : 0xAAAAAA00, 0x00000000);
             }
        }
    }
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
