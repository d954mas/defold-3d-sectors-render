#pragma once
#include <dmsdk/sdk.h>
#include <vector>

struct xy {float x,y;};
struct xyz {float x,y,z;};
struct sector
{
    float floor, ceil;
    std::vector<int> vertex; //vertex have x,y coords
    std::vector<int> neighbors; //Each edge may have a corresponding neighboring sector
};
struct player{
    struct xyz where, velocity;
    float angle, anglesin, anglecos, yaw;//looking towards(sin() and cos() thereof) yaw is rotation of player?
    int sector; //current sector
};

void setBuffer(int, int, dmScript::LuaHBuffer*);

void clearBuffer();

void DrawScreen();
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
