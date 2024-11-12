#pragma once

#include "types/vector2/vector2_t.hpp"
#include "Map.hpp"
// #include "drivers/IMU/"


namespace gxm{


class Navigator{
    struct Config{

    };
protected:
    const Config & config_;
    const Map & map_;

    Vector2 curr_pos;

public:
    Navigator(const Config & config, const Map & map):
        config_(config), map_(map){}

    void recalibrate(const Area area, const Vector2 & offs){
        curr_pos = area.to_pos(map_) + offs;
    }

    void update(){
        
    }
};

}