#include "minimap.h"
#include "buffer.h"

static Buffer minimapBuffer;

static uint32_t PLAYER_COLOR = 0xFF880000;

void MinimapUpdate(){
    printf("minimap update\n");
    MinimapClearBuffer();
    int centerX= minimapBuffer.width/2, centerY = minimapBuffer.height/2;
    //draw player at center
    int size = 10;
    DrawRect(minimapBuffer, centerX-size/2, centerY-size/2, size,size,PLAYER_COLOR);
}

void MinimapSetBuffer(int width, int height, dmScript::LuaHBuffer* luaBuffer){
 	minimapBuffer = CreateBuffer(width, height, luaBuffer);
}

void MinimapClearBuffer(){
	ClearBuffer(minimapBuffer);
}




