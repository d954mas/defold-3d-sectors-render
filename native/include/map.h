#pragma once
#include <dmsdk/sdk.h>

//map loading
void MapClear();
void MapVertexAdd(float, float);
void MapVertexChange(int,float,float);

void MapSectorCreate(float, float);
//add to last sector in list
void MapSectorVertexAdd(int,int);
//check that map valid
void MapCheck();

void MapSectorChangeFloor(int,float);
void MapSectorChangeCeil(int,float);
void MapBind(lua_State*);



