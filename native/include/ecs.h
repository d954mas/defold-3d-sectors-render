#pragma once
#include <entityx/entityx.h>

struct PositionC {
  PositionC(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}
  float x, y, z;
};

struct VelocityC {
  VelocityC(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}
  float x, y, z;
};

struct MovementSpeedC{
    MovementSpeedC(float v = 0.0f) : v(v) {}
    float v;
};

struct HandleCollisionC {
      HandleCollisionC(float dx = 0.0f, float dy = 0.0f, float dz = 0.0f) : dx(dx), dy(dy), dz(dz) {}
      float dx, dy, dz;
};


//HeadMargin
struct HeadMarginC{
  HeadMarginC(float v=0) : v(v) {}
  float v;
};

//Camera height from floor when standing
struct EyeHeightC{
  EyeHeightC(float v=0) : v(v) {}
  float v;
};
//How tall obstacles the player can simply walk over without jumping
struct KneeHeightC{
  KneeHeightC(float v=0) : v(v) {}
  float v;
};

struct AngleC{
    AngleC(float angle1=0){
        setAngle(angle1);
    }
    void setAngle(float angle1)
    {
         angle = angle1;
         anglesin = sin(angle);
         anglecos = cos(angle);
    }
    float angle, anglesin, anglecos;
};

struct SectorC{
    SectorC(int v=0) : v(v) {}
    int v;
};

//region systems


struct MovementSystem : public entityx::System<MovementSystem> {
  void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override {
    es.each<PositionC, VelocityC, MovementSpeedC>([dt](entityx::Entity entity, PositionC &position, VelocityC &vel, MovementSpeedC &speed) {
        float dx =  vel.x * speed.v * dt;
        float dy =  vel.y * speed.v * dt;
        float dz =  vel.z * speed.v * dt;
        if (entity.has_component<HandleCollisionC>()){
            entityx::ComponentHandle<HandleCollisionC> collision = entity.component<HandleCollisionC>();
            collision->dx = dx;
            collision->dy = dy;
            collision->dz = dz;
        }else{
            position.x += dx;
            position.y += dy;
            position.z += dz;
        }

    });
  };
};

//endregion



struct xy {float x,y;};
struct xyz {float x,y,z;};
/* Sectors: Floor and ceiling height; list of edge vertices and neighbors */
//Sector is a room, where i can set floor and ceiling height
//Sector can be 2 types. Wall and portal. We can see throw portal
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

struct World;

class EcsWorld : public entityx::EntityX {

public:
    World *world;
    explicit EcsWorld() {
        systems.add<MovementSystem>();
        systems.configure();
    }

    void setWorld(World* var){
        world = var;
    }

    void update(entityx::TimeDelta dt) {
     //   systems.update<MovementSystem>(dt);
    }

    void reset(){
        entities.reset();
    }

};

struct World{
    EcsWorld ecs;
    std::vector<struct sector> sectors;
    std::vector<struct xy> vertices;
    player player;

    void reset(){
        sectors.clear();
        vertices.clear();
        player = (struct player) { {0,0,0}, {0,0,0}, 0,0,0,0, 0 };
        ecs.reset();
    }
};

extern World world;


static void WorldUpdate(float dt){
    world.ecs.update(dt);
}
  