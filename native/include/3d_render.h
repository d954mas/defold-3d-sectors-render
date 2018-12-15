#pragma once
#include <dmsdk/sdk.h>

void LoadLevel( char *, uint32_t);
void UnloadLevel();
void setBuffer(int, int, dmScript::LuaHBuffer*);

void clearBuffer();

void DrawScreen();
void MovePlayer(float,float);
void GetPlayerPos(float *, float *, float *);
void SetAngle(float angle);
void SetYaw(float yaw);