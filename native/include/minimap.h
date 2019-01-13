#pragma once
#include <dmsdk/sdk.h>
#include <vector>
#include "ecs.h"

void MinimapUpdate();
void MinimapSetBuffer(int, int, dmScript::LuaHBuffer*);
void MinimapClearBuffer();
void MinimapSetPlayerSizePixels(int);
void MinimapSetMapSize(int, int);
void MinimapBind(lua_State*);
