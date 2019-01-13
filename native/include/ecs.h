#pragma once
#include <entityx/entityx.h>
#include "world.h"

//region COMPONENTS
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

struct HandleGravityC {
      HandleGravityC(bool falling = true) : falling(falling){}
      bool falling;
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

struct YawC{
    YawC(float yaw1=0){
        setYaw(yaw1);
    }
    void setYaw(float yaw1)
    {
        yaw = yaw1;
        yawsin = sin(yaw);
        yawcos = cos(yaw);
    }
    float yaw, yawsin, yawcos;
};

struct SectorC{
    SectorC(int v=0) : v(v) {}
    int v;
};

//region systems
struct MovementSystem : public entityx::System<MovementSystem> {
  void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override {
    es.each<PositionC, VelocityC,AngleC, MovementSpeedC>([dt](entityx::Entity entity, PositionC &position, VelocityC &vel,AngleC &angle, MovementSpeedC &speed) {
        float vx = vel.x * angle.anglecos - vel.y * angle.anglesin;
        float vy = vel.x * angle.anglesin + vel.y * angle.anglecos;
        float dx =  vx * speed.v * dt;
        float dy =  vy * speed.v * dt;
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

struct CollisionSystem : public entityx::System<CollisionSystem> {
  void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override {
    es.each<PositionC,VelocityC,HandleCollisionC, EyeHeightC, SectorC,HeadMarginC,KneeHeightC>([dt](entityx::Entity entity, PositionC &position, VelocityC &vel, HandleCollisionC &col,
    EyeHeightC &eye, SectorC &sector,HeadMarginC &head,KneeHeightC &knee) {
        position.z += col.dz;
        const Sector &sect = WORLD.sectors[sector.v];
        const std::vector<int> &vert = sect.vertex;
        if(col.dx != 0 || col.dy !=0){
             for(unsigned s = 0; s < vert.size()-1; ++s){
                XY v =  WORLD.vertices[vert[s+0]];
                XY v2 =  WORLD.vertices[vert[s+1]];
                    if(IntersectBox(position.x, position.y, position.x+col.dx,position.y+col.dy, v.x, v.y, v2.x, v2.y)
                        && PointSide(position.x+col.dx, position.y+col.dy, v.x, v.y, v2.x, v2.y) < 0){
                        //Check where the hole is.
                        float hole_low  = sect.neighbors[s] < 0 ?  9e9 : max(sect.floor, WORLD.sectors[sect.neighbors[s]].floor);
                        float hole_high = sect.neighbors[s] < 0 ? -9e9 : min(sect.ceil,  WORLD.sectors[sect.neighbors[s]].ceil );
                        // Check whether we're bumping into a wall.
                        if(hole_high < position.z+eye.v+head.v
                            || hole_low  >position.z+knee.v){
                            // Bumps into a wall! Slide along the wall.
                            // This formula is from Wikipedia article "vector projection".
                            float xd = v2.x - v.x, yd = v2.y - v.y;
                            col.dx = xd * (col.dx*xd + yd*col.dy) / (xd*xd + yd*yd);
                            col.dy = yd * (col.dx*xd + yd*col.dy) / (xd*xd + yd*yd);
                        }
                    }
             }
              for(unsigned s = 0; s < vert.size()-1; ++s){
                XY v =  WORLD.vertices[vert[s+0]];
                XY v2 =  WORLD.vertices[vert[s+1]];
                     if(sect.neighbors[s] >= 0 && IntersectBox(position.x,position.y, position.x+col.dx,position.y+col.dy, v.x, v.y, v2.x, v2.y)
                                            && PointSide(position.x+col.dx, position.y+col.dy, v.x, v.y, v2.x, v2.y) < 0){
                         sector.v = sect.neighbors[s];
                         break;
                     }
              }
              position.x += col.dx;
              position.y += col.dy;
              col.dx = 0;col.dy = 0;
        }

        if(entity.has_component<HandleGravityC>()){
           entityx::ComponentHandle<HandleGravityC> grav = entity.component<HandleGravityC>();
           grav->falling = true;
           if(grav->falling){
                vel.z -= 0.05f*60 * dt;
                float nextz = position.z + vel.z;
                if(vel.z < 0 && nextz  < WORLD.sectors[sector.v].floor){
                    position.z = WORLD.sectors[sector.v].floor;
                    vel.z = 0;
                    grav->falling = false;
                }else if(vel.z > 0 && nextz > WORLD.sectors[sector.v].ceil-eye.v){
                    vel.z = 0;
                    grav->falling = true;
                }if(grav->falling){
                    position.z += vel.z;
                }
           }
        }
    });
  };
};
//endregion


entityx::Entity checkEntity (lua_State*, int);
