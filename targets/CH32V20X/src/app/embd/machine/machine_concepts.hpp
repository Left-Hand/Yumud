#pragma once

#include "../../sys/core/platform.h"
#include "../../types/vector2/vector2_t.hpp"


struct XY_Machine{
    virtual void x_mm(const real_t x) = 0;
    virtual void y_mm(const real_t y) = 0;
public:

    void xy_mm(const real_t x, const real_t y){
        x_mm(x);
        y_mm(y);
    }
    void xy_mm(const Vector2 & v){
        xy_mm(v.x, v.y);
    }
};

struct Z_Machine{
    virtual void z_mm(const real_t z) = 0;
};

struct XYZ_Machine:public XY_Machine, public Z_Machine{

};

struct Nozzle{

};

struct Nozzle_Machine{

};

struct CoreXY{

};

struct Cantilever:public XYZ_Machine{

};