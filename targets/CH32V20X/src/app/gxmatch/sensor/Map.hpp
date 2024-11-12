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
    Vector2 garbage_pos;
    Vector2 billboard_pos;
    Vector2 staging_pos;
    Vector2 raw_material_pos;
    Vector2 rough_process_pos;

    Map & operator =(const Map & other) = default;
    Map & operator =(Map && other) = default;
};

static inline Map map;

class Area{
public:
protected:
    AreaType type_;

    const Map & map_ = map;
public:
    Area(AreaType type):type_(type){}
    AreaType type() const{return type_;}
    Area & operator = (const AreaType type){type_ = type; return *this;}
    bool operator ==(const AreaType type) const {return type_ == type;}
    bool operator !=(const AreaType type) const {return type_ != type;}

    explicit operator Vector2() const;
};






}