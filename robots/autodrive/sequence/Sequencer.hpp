#pragma once

#include "SequenceUtils.hpp"

namespace ymd::robots{

//路径序列化器 将几何路径渲染为位姿集
class Sequencer final{
protected:
    const SequenceLimits & limits_;
    const SequenceParas & paras_;

    void rotate(Curve & curve, const math::Ray2<iq16> & from, const Angular<iq16> end_angle);

    void linear(Curve & curve, const math::Ray2<iq16> & from, const math::Vec2<iq16> & end_pos);
public:
    Sequencer(const SequenceLimits & limits, const SequenceParas & paras):
        limits_(limits), paras_(paras){}
    
    //圆弧插补 未实现
    void arc(Curve & curve, const math::Ray2<iq16> & from, const math::Ray2<iq16> & to, const iq16 radius);
    
    //侧向移动
    void sideways(Curve & curve, const math::Ray2<iq16> & from, const math::Ray2<iq16> & to);

    //径向移动
    void follow(Curve & curve, const math::Ray2<iq16> & from, const math::Ray2<iq16> & to);

    //平移
    void shift(Curve & curve, const math::Ray2<iq16> & from, const math::Ray2<iq16> & to);

    //旋转
    void spin(Curve & curve, const math::Ray2<iq16> & from, const math::Ray2<iq16> & to);

    void wait(Curve & curve, const math::Ray2<iq16> & from, const iq16 dur);
};
}