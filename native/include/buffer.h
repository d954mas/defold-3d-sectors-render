#pragma once
#include <dmsdk/sdk.h>

struct Buffer {
	int width;
	int height;
	uint8_t* stream;
};

Buffer CreateBuffer(int, int, dmScript::LuaHBuffer*);

void ClearBuffer(Buffer &buffer);

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
