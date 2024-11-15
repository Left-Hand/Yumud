#pragma once

#include "types/vector2/vector2_t.hpp"
#include "Planner.hpp"
// #include "drivers/IMU/"


namespace gxm{

//机器人导航的顶层调度类
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

    void recalibrate(const Field field, const Vector2 & offs){
        curr_pos = field.to_pos(map_) + offs;
    }

    void update(){
        
    }
};

}