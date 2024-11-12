#pragma once

#include "types/vector2/vector2_t.hpp"


namespace gxm{

using Vector2 = yumud::Vector2_t<real_t>;

enum class AreaType:uint8_t{
    Garbage,
    Billboard,
    Staging,
    RawMaterial,
    RoughProcess
};

struct Map{
    Vector2 garbage_pos = {0,0};
    Vector2 billboard_pos = {real_t(0.065), real_t(0.3)};
    Vector2 staging_pos = {real_t(1.8), real_t(1.015)};
    Vector2 raw_material_pos = {real_t(1.45), real_t(0.3)};
    Vector2 rough_process_pos = {real_t(1.05), real_t(1.72)};

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

    Vector2 to_pos(const Map & map_) const;
};


}