#pragma once
#include <dmsdk/sdk.h>
#include <vector>
#include "ecs.h"


void RenderClearBuffer();
void RenderSetBuffer(int, int, dmScript::LuaHBuffer*);

void RenderDrawScreen(entityx::Entity);

void RenderBind(lua_State*);



