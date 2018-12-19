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
    float v, y, z;
};

struct HandleGravityC {};


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
      position.x += vel.x * speed.v * dt;
      position.y += vel.y * speed.v * dt;
      position.z += vel.z * speed.v * dt;
    });
  };
};

//endregion

class EcsWorld : public entityx::EntityX {
public:
  explicit EcsWorld() {
    systems.add<MovementSystem>();
   // systems.add<MovementSystem>();
   // systems.add<CollisionSystem>();
    systems.configure();

   // level.load(filename);

   // for (auto e : level.entity_data()) {
    //  entityx::Entity entity = entities.create();
  //    entity.assign<Position>(rand() % 100, rand() % 100);
  //    entity.assign<Direction>((rand() % 10) - 5, (rand() % 10) - 5);
   // }
  }

  void update(entityx::TimeDelta dt) {
  //  systems.update<DebugSystem>(dt);
    systems.update<MovementSystem>(dt);
   // systems.update<CollisionSystem>(dt);
  }
};
  