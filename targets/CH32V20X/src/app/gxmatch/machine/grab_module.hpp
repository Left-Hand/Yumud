#pragma once

#include "motion_module.hpp"

namespace gxm{

class GrabModule:public MotionModule{


protected:
    struct Config{
        uint8_t tray_height_mm;
        uint8_t free_height_mm;
        uint8_t ground_height_mm;
    };

    void goHome();//进行坐标归位
    void moveZ(const real_t pos);//只改变Z轴坐标
    void moveXY(const Vector2 & pos);//只改变XY坐标
    void moveTo(const Vector3 & pos);//改变所有坐标

    void pickUp();//拾起物块
    void putDown();//放下物块

    // Vector2 calculatePos(TrayIndex index)
public:
    void take();
    void give();
    bool done();
    void begin();
};
    
}