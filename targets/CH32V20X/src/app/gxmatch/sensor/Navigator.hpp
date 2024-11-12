#pragma once

#include "types/vector2/vector2_t.hpp"
#include "Map.hpp"
// #include "drivers/IMU/"


namespace gxm{


class Navigator{
    struct Config{

    };
protected:
    const Map & map_ = map;
    const Config & config_;

    Vector2 curr_pos;

public:
    Navigator(const Config & config):
        config_(config){}

    void recalibrate(const Area area, const Vector2 & offs){
        curr_pos = Vector2(area) + offs;
    }

    void update(){
        
    }
};

}