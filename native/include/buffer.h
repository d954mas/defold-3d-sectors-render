#pragma once
#include <dmsdk/sdk.h>

struct Buffer {
	int width;
	int height;
	uint8_t* stream;
} pixelBuffer;

Buffer createBuffer(int width, int height, dmScript::LuaHBuffer* luaBuffer){
    Buffer buffer;
	buffer.width = width;
	buffer.height = height;
	dmBuffer::HBuffer hBuffer = luaBuffer->m_Buffer;
	uint32_t size_ignored = 0;
	dmBuffer::Result r = dmBuffer::GetBytes(hBuffer, (void**)&buffer.stream, &size_ignored);
    return buffer;
}

void clearBuffer(Buffer &buffer){
	int size = buffer.width * buffer.height;
	for(int i=0; i< size*3; i+=3){
		buffer.stream[i] = 0;
		buffer.stream[i+1] = 0;
		buffer.stream[i+2] = 0;
	}
}

inline void DrawPixel(Buffer &b,int x, int y, uint32_t color){
	uint8_t red = (color & 0xFF000000) >> 24;
	uint8_t green = (color & 0x00FF0000) >> 16;
	uint8_t blue = (color & 0x0000FF00) >> 8;
    int start = ((b.height - y - 1) * b.width + x)*3;
    b.stream[start] = red;
    b.stream[start+1] = green;
    b.stream[start+2] = blue;
}
