#pragma once
#include <entityx/entityx.h>
#include "world.h"

#define EPS 0.01

//region COMPONENTS
struct PositionC {
  PositionC(vec2f pos=vec2f(), float z = 0.0f) : pos(pos), z(z) {}
  vec2f pos; float z;
};

struct VelocityC {
  VelocityC(vec2f pos=vec2f(), float z = 0.0f) : pos(pos), z(z) {}
  vec2f pos; float z;
};

struct MovementSpeedC{
    MovementSpeedC(float v = 0.0f) : v(v) {}
    float v;
};

struct HandleCollisionC {
    HandleCollisionC(vec2f dpos=vec2f(), float dz = 0.0f) : dpos(dpos), dz(dz) {}
    vec2f dpos; float dz;
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
        vec2f pos = vel.pos.rotate(-angle.angle) * speed.v * dt;
        float dz =  vel.z * speed.v * dt;
        if (entity.has_component<HandleCollisionC>()){
            entityx::ComponentHandle<HandleCollisionC> collision = entity.component<HandleCollisionC>();
            collision->dpos = pos;
            collision->dz = dz;
        }else{
            position.pos += pos;
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
        if(col.dpos.x != 0 || col.dpos.y !=0){
            vec2f dcol = vec2f(0,0);
            printf("***************************************************\n");
            printf("collide from s:%d\n",sector.v);
            for(unsigned s = 0; s < vert.size()-1; ++s){
                vec2f v1 =  WORLD.vertices[vert[s]];
                vec2f v2 =  WORLD.vertices[vert[s+1]];
                vec2f line = v2-v1;
                vec2f playerEnd = position.pos + col.dpos;
                vec2f normal =line.ortho().normalize();

                int neighbor = sect.neighbors[s];
                 printf("collide neighbor:%d(%d)\n",s,neighbor);
                 printf("collide line:(%f;%f)(%f;%f)\n", v1.x, v1.y,v2.x, v2.y);
                float d =vec2f::dot(normal, v1);// The closest distance to the line from the origin (0, 0), is in the direction of the normal
                // Check the distance from the line to the player start position
                float startDist = vec2f::dot(normal, position.pos) - d;
                // If the distance is negative, that means the player is 'behind' the line
                // To correctly use the normal, if that is the case, invert the normal
                if(startDist < 0.0f) {
                    normal = vec2f(-normal.x,-normal.y);
                    d = -d;
                }
                // Check the distance from the line to the player end position
                // (using corrected normal if necessary, so playerStart is always in front of the line now)
                float endDist = vec2f::dot(normal, playerEnd) - d;

                // Check if playerEnd is behind the line COLLISION
                if(endDist < 0.0f && IntersectBox(position.pos.x,position.pos.y, position.pos.x+col.dpos.x,position.pos.y+col.dpos.y, v1.x, v1.y, v2.x, v2.y)){
                    float hole_low  = neighbor < 0 ?  9e9 : max(sect.floor, WORLD.sectors[sect.neighbors[s]].floor);
                    float hole_high = neighbor < 0 ? -9e9 : min(sect.ceil,  WORLD.sectors[sect.neighbors[s]].ceil );
                    // Check whether we're bumping into a wall.
                    if(hole_high < position.z+eye.v+head.v|| hole_low  >position.z+knee.v){
                        // Calculate the new position by moving playerEnd out to the line in the direction of the normal,
                        // and a little bit further to counteract floating point inaccuracies
                        // eps should be something less than a visible pixel, so it's not noticeable
                        printf("collision\n");
                        vec2f move = normal * (-endDist + EPS);
                        col.dpos = (playerEnd + move)-position.pos;
                    }
                }
            }
             for(unsigned s = 0; s < vert.size()-1; ++s){
                            vec2f v =  WORLD.vertices[vert[s+0]];
                            vec2f v2 =  WORLD.vertices[vert[s+1]];
                                 if(sect.neighbors[s] >= 0 && IntersectBox(position.pos.x,position.pos.y, position.pos.x+col.dpos.x,position.pos.y+col.dpos.y, v.x, v.y, v2.x, v2.y)
                                                        && PointSide(position.pos.x+col.dpos.x, position.pos.y+col.dpos.y, v.x, v.y, v2.x, v2.y) < 0){
                                     sector.v = sect.neighbors[s];
                                    // printf("change sector:%d\n",  sect.neighbors[s]);
                                     break;
                                 }
                          }
         position.pos += col.dpos;

        }
        col.dpos.x = 0;col.dpos.y = 0;col.dz = 0;

        //check collisions z. update velocity z
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
                //    position.z += vel.z;
                }
           }
        }
    });
  };
};
//endregion


entityx::Entity checkEntity (lua_State*, int);
