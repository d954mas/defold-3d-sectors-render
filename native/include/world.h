#pragma once
#include <dmsdk/sdk.h>
#include <entityx/entityx.h>
#include <vector>
#include "vec.h"
/*
fix black sectors
Ok, I kinda fixed it, but it feels more like a hack than a fix to me.
 Anyway, here's what I did: -
  I changed the Intersect function to return null if the line segments don't cross each other;
   - As a side effect of the previous one, I had to make sure that the tx* and tz*
        only changed if the i1 and i2 vertices were not null (in the clipping part);
   - Last but not least, before we calculate the x1 and x2 values,
   I added this boundary check: if (tz1 < 0.1) tz1= 0.1f; if (tz2 < 0.1) tz2 = 0.1f;

   Everything seems to be working fine as of right now, but I didn't have the opportunity to do much checking yet.﻿
/*
/* Define various vision related constants */
#define hfovm (W * WORLD.hfov * 0.73f*H/W)
#define vfovm (H * WORLD.vfov * .2f)

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
#define Intersect(x1,y1, x2,y2, x3,y3, x4,y4) vec2f( \
    vxs(vxs(x1,y1, x2,y2), (x1)-(x2), vxs(x3,y3, x4,y4), (x3)-(x4)) / vxs((x1)-(x2), (y1)-(y2), (x3)-(x4), (y3)-(y4)), \
    vxs(vxs(x1,y1, x2,y2), (y1)-(y2), vxs(x3,y3, x4,y4), (y3)-(y4)) / vxs((x1)-(x2), (y1)-(y2), (x3)-(x4), (y3)-(y4)))


/* Sectors: Floor and ceiling height; list of edge vertices and neighbors */
//Sector is a room, where i can set floor and ceiling height
//Sector can be 2 types. Wall and portal. We can see throw portal
struct Sector{
    float floor, ceil;
    std::vector<int> vertex; //vertex have x,y coords
    std::vector<int> neighbors; //Each edge may have a corresponding neighboring sector
};

class EcsWorld : public entityx::EntityX {
public:
    explicit EcsWorld();
    void update(entityx::TimeDelta dt);
    void reset();
};
extern EcsWorld ECS;

struct World{
    float hfov=1, vfov=1;
    std::vector<Sector> sectors;
    std::vector<vec2f> vertices;
    void reset();
    void setHFov(float);
    void setVFov(float);
};
extern World WORLD;

void WorldUpdate(float);

void WorldBind(lua_State*);

bool IsInside(Sector&, vec2f&);

bool Intersect2(vec2f,vec2f,vec2f,vec2f, vec2f&);
bool Intersect4(vec2f,vec2f,vec2f,vec2f, vec2f&);