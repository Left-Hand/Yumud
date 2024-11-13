#pragma once

#include "Gesture2D.hpp"

namespace gxm{

enum class AreaType:uint8_t{
    Garbage,
    Billboard,
    Staging,
    RawMaterial,
    RoughProcess
};

struct Map{
    Gesture2D garbage_gest =           {real_t(0),       real_t(0),        real_t(PI/2)};
    Gesture2D billboard_gest =         {real_t(0.065),   real_t(0.3),      real_t(-PI/2)};
    Gesture2D staging_gest =           {real_t(1.8),     real_t(1.015),    real_t(0)};
    Gesture2D raw_material_gest =      {real_t(1.45),    real_t(0.3),      real_t(-PI/2)};
    Gesture2D rough_process_gest =     {real_t(1.05),    real_t(1.72),     real_t(PI/2)};

    Map & operator =(const Map & other) = default;
    Map & operator =(Map && other) = default;
};

class Area{
public:
protected:
    AreaType type_;

public:
    Area(AreaType type):type_(type){}
    AreaType type() const{return type_;}
    Area & operator = (const AreaType type){type_ = type; return *this;}
    bool operator ==(const AreaType type) const {return type_ == type;}
    bool operator !=(const AreaType type) const {return type_ != type;}

    Vector2 to_pos(const Map & map) const;
    real_t to_rot(const Map & map) const;
};


}