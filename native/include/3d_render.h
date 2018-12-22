#pragma once
#include <dmsdk/sdk.h>
#include <vector>
#include "ecs.h"


void RenderClearBuffer();
void RenderSetBuffer(int, int, dmScript::LuaHBuffer*);

void DrawScreen(entityx::Entity);
void PlayerInit(int,float,float);
void MovePlayer(float,float);
void GetPlayerPos(float *, float *, float *);
void SetAngle(float angle);
void SetYaw(float yaw);


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

//region Player

void PlayerInit(int,float,float);


