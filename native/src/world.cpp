#include <dmsdk/sdk.h>
#include "world.h"

void WorldUpdate(float dt){
     ECS.update(dt);
}

void World::reset(){
    sectors.clear();
    vertices.clear();
    ECS.reset();
}

World WORLD;
EcsWorld ECS;
