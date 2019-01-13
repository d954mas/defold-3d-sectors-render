#pragma once
#include <dmsdk/sdk.h>

#define min(a,b)             (((a) < (b)) ? (a) : (b)) // min: Choose smaller of two scalars.
#define max(a,b)             (((a) > (b)) ? (a) : (b)) // max: Choose greater of two scalars.
#define clamp(a, mi,ma)      min(max(a,mi),ma)         // clamp: Clamp value into set range.

struct Buffer {
	int width;
	int height;
	uint8_t* stream;
};

Buffer CreateBuffer(int, int, dmScript::LuaHBuffer*);

void ClearBuffer(Buffer &buffer);

static inline bool BufferPointIn(Buffer &b,int x, int y){
	return x>=0 && x < b.width && y>=0 && y< b.height;
}

//not worked. Line can be in screen when it edges not
//-static inline bool BufferLineIn(Buffer &b,int x1, int y1,int x2, int y2){
   // return BufferPointIn(b,x1,y1) || BufferPointIn(b,x2,y2);
//}

static inline void BufferClampPoint(Buffer &b,int* x1, int*y1){
    *x1 = clamp(*x1,0,b.width-1);
    *y1 = clamp(*y1,0,b.height-1);
}

static inline void DrawPixel(Buffer &b,int x, int y, uint32_t color){
	uint8_t red = (color & 0xFF000000) >> 24;
	uint8_t green = (color & 0x00FF0000) >> 16;
	uint8_t blue = (color & 0x0000FF00) >> 8;
    int start = ((b.height - y - 1) * b.width + x)*3;
    b.stream[start] = red;
    b.stream[start+1] = green;
    b.stream[start+2] = blue;
}

static inline void DrawRect(Buffer &b,int x, int y,int w, int h, uint32_t color){
    for (int yi = 0; yi< h; yi++){
        for (int xi = 0; xi< w; xi++){
            DrawPixel(b,x + xi,y + yi,color);
        }
    }
}

static inline void DrawLine(Buffer &b,int x0, int y0,int x1, int y1, uint32_t color, bool inverted){
        int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2; /* error value e_xy */
        uint8_t red = (color & 0xFF000000) >> 24;
        uint8_t green = (color & 0x00FF0000) >> 16;
        uint8_t blue = (color & 0x0000FF00) >> 8;
        bool pointInside = false;
        for (;;){  /* loop */
            if (BufferPointIn(b,x0,y0)){
                int start = (((inverted ?  y0 : (b.height - y0 - 1))*b.width) + x0)*3;
                b.stream[start] = red;
                b.stream[start+1] = green;
                b.stream[start+2] = blue;
                pointInside = true;
            }else if(pointInside){
                break;
            }
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
            if (e2 <= dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
        }
}
