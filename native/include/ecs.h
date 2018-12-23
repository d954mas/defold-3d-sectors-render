#pragma once
#include <entityx/entityx.h>

/* Define window size */
/* Define various vision related constants */
#define EyeHeight  6    // Camera height from floor when standing
#define HeadMargin 1    // How much room there is above camera before the head hits the ceiling
#define KneeHeight 2    // How tall obstacles the player can simply walk over without jumping
#define hfov (0.73f*H)  // Affects the horizontal field of vision
#define vfov (.2f*H)    // Affects the vertical field of vision

// Utility functions. Because C doesn't have templates,
// we use the slightly less safe preprocessor macros to
// implement these functions that work with multiple types.
#define min(a,b)             (((a) < (b)) ? (a) : (b)) // min: Choose smaller of two scalars.
#define max(a,b)             (((a) > (b)) ? (a) : (b)) // max: Choose greater of two scalars.
#define clamp(a, mi,ma)      min(max(a,mi),ma)         // clamp: Clamp value into set range.
#define vxs(x0,y0, x1,y1)    ((x0)*(y1) - (x1)*(y0))   // vxs: Vector cross product
// Overlap:  Determine whether the two number ranges overlap.
#define Overlap(a0,a1,b0,b1) (min(a0,a1) <= max(b0,b1) && min(b0,b1) <= max(a0,a1))
// IntersectBox: Determine whether two 2D-boxes intersect.
#define IntersectBox(x0,y0, x1,y1, x2,y2, x3,y3) (Overlap(x0,x1,x2,x3) && Overlap(y0,y1,y2,y3))
// PointSide: Determine which side of a line the point is on. Return value: <0, =0 or >0.
#define PointSide(px,py, x0,y0, x1,y1) vxs((x1)-(x0), (y1)-(y0), (px)-(x0), (py)-(y0))
// Intersect: Calculate the point of intersection between two lines.
#define Intersect(x1,y1, x2,y2, x3,y3, x4,y4) ((XY) { \
    vxs(vxs(x1,y1, x2,y2), (x1)-(x2), vxs(x3,y3, x4,y4), (x3)-(x4)) / vxs((x1)-(x2), (y1)-(y2), (x3)-(x4), (y3)-(y4)), \
    vxs(vxs(x1,y1, x2,y2), (y1)-(y2), vxs(x3,y3, x4,y4), (y3)-(y4)) / vxs((x1)-(x2), (y1)-(y2), (x3)-(x4), (y3)-(y4)) })

struct XY {float x,y;};
struct XYZ {float x,y,z;};
/* Sectors: Floor and ceiling height; list of edge vertices and neighbors */
//Sector is a room, where i can set floor and ceiling height
//Sector can be 2 types. Wall and portal. We can see throw portal
struct Sector{
    float floor, ceil;
    std::vector<int> vertex; //vertex have x,y coords
    std::vector<int> neighbors; //Each edge may have a corresponding neighboring sector
};

struct World;
extern World world;

class EcsWorld : public entityx::EntityX {
public:
    World *world;
    explicit EcsWorld();

    void setWorld(World* var){
        world = var;
    }

    void update(entityx::TimeDelta dt);

    void reset(){
        entities.reset();
    }
};

struct World{
    EcsWorld ecs;
    std::vector<Sector> sectors;
    std::vector<XY> vertices;

    void reset(){
        sectors.clear();
        vertices.clear();
        ecs.reset();
    }
};




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

/*void MovePlayer(float x, float y){
    float eyeheight =  EyeHeight;
    const sector &sect = world.sectors[world.player.sector];
    const std::vector<int>  vert = sect.vertex;
    float px = world.player.where.x, py = world.player.where.y;
    float dx = x - world.player.where.x, dy =y - world.player.where.y;

    //vertical
     /* Vertical collision detection */
    /*ground = !falling;
    if(falling){
        world.player.velocity.z -= 0.05f; /* Add gravity
        float nextz = world.player.where.z + world.player.velocity.z;
        if(world.player.velocity.z < 0 && nextz  < world.sectors[world.player.sector].floor + eyeheight) // When going down
        {
            /* Fix to ground
            world.player.where.z    = world.sectors[world.player.sector].floor + eyeheight;
            world.player.velocity.z = 0;
            falling = 0;
            ground  = 1;
        }
        else if(world.player.velocity.z > 0 && nextz > world.sectors[world.player.sector].ceil) // When going up
        {
            /* Prevent jumping above ceiling
            world.player.velocity.z = 0;
            falling = 1;
        }
        if(falling)
        {
            world.player.where.z += world.player.velocity.z;
            moving = 1;
        }
    }

    //horizontal


    for(unsigned s = 0; s < vert.size()-1; ++s){
        xy v =  world.vertices[vert[s+0]];
        xy v2 =  world.vertices[vert[s+1]];
        if(IntersectBox(px,py, px+dx,py+dy, v.x, v.y, v2.x, v2.y)
            && PointSide(px+dx, py+dy, v.x, v.y, v2.x, v2.y) < 0){
            /* Check where the hole is.
            float hole_low  = sect.neighbors[s] < 0 ?  9e9 : max(sect.floor, world.sectors[sect.neighbors[s]].floor);
            float hole_high = sect.neighbors[s] < 0 ? -9e9 : min(sect.ceil,  world.sectors[sect.neighbors[s]].ceil );
            /* Check whether we're bumping into a wall.
            if(hole_high < world.player.where.z+HeadMargin
                || hole_low  > world.player.where.z-eyeheight+KneeHeight){
                /* Bumps into a wall! Slide along the wall.
                /* This formula is from Wikipedia article "vector projection".
                float xd = v2.x - v.x, yd = v2.y - v.y;
                dx = xd * (dx*xd + yd*dy) / (xd*xd + yd*yd);
                dy = yd * (dx*xd + yd*dy) / (xd*xd + yd*yd);
            }
        }
    }
    falling = 1;
    for(unsigned s = 0; s < vert.size()-1; ++s){
        xy v1 =  world.vertices[vert[s+0]];
        xy v2 =  world.vertices[vert[s+1]];
        if(sect.neighbors[s] >= 0
        && IntersectBox(px,py, px+dx,py+dy, v1.x, v1.y, v2.x, v2.y)
        && PointSide(px+dx, py+dy, v1.x,v1.y, v2.x, v2.y) < 0)
        {
            world.player.sector = sect.neighbors[s];
            break;
        }
    }

    world.player.where.x = px + dx;
    world.player.where.y = py + dy;*/
    //}
    
struct CollisionSystem : public entityx::System<CollisionSystem> {
  void update(entityx::EntityManager &es, entityx::EventManager &events, entityx::TimeDelta dt) override {
    es.each<PositionC,HandleCollisionC, EyeHeightC, SectorC,HeadMarginC,KneeHeightC>([dt](entityx::Entity entity, PositionC &position, HandleCollisionC &col,
    EyeHeightC &eye, SectorC &sector,HeadMarginC &head,KneeHeightC &knee) {
        position.z += col.dz;
        const Sector &sect = world.sectors[sector.v];
        const std::vector<int> vert = sect.vertex;
        if(col.dx != 0 || col.dy !=0){
             for(unsigned s = 0; s < vert.size()-1; ++s){
                    XY v =  world.vertices[vert[s+0]];
                    XY v2 =  world.vertices[vert[s+1]];
                    if(IntersectBox(position.x, position.y, position.x+col.dx,position.y+col.dy, v.x, v.y, v2.x, v2.y)
                        && PointSide(position.x+col.dx, position.y+col.dy, v.x, v.y, v2.x, v2.y) < 0){
                        //Check where the hole is.
                        float hole_low  = sect.neighbors[s] < 0 ?  9e9 : max(sect.floor, world.sectors[sect.neighbors[s]].floor);
                        float hole_high = sect.neighbors[s] < 0 ? -9e9 : min(sect.ceil,  world.sectors[sect.neighbors[s]].ceil );
                        // Check whether we're bumping into a wall.
                        if(hole_high < position.z+head.v
                            || hole_low  >position.z-eye.v+knee.v){
                            // Bumps into a wall! Slide along the wall.
                            // This formula is from Wikipedia article "vector projection".
                            float xd = v2.x - v.x, yd = v2.y - v.y;
                            col.dx = xd * (col.dx*xd + yd*col.dy) / (xd*xd + yd*yd);
                            col.dy = yd * (col.dx*xd + yd*col.dy) / (xd*xd + yd*yd);
                        }
                    }
             }
              for(unsigned s = 0; s < vert.size()-1; ++s){
                     XY v =  world.vertices[vert[s+0]];
                     XY v2 =  world.vertices[vert[s+1]];
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
       col.dz = 0;
    });
  };
};

//endregion




static void WorldUpdate(float dt){
    world.ecs.update(dt);
}
  