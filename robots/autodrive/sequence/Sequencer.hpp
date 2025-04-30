#pragma once

#include "SequenceUtils.hpp"
#include "SequencerBase.hpp"

namespace ymd::robots{

//路径序列化器 将几何路径渲染为位姿集
class Sequencer:public SequencerBase{
protected:
    const SequenceLimits & limits_;
    const SequenceParas & paras_;

    void rotate(Curve & curve, const Ray2_t<real_t> & from, const real_t & end_rad);

    void linear(Curve & curve, const Ray2_t<real_t> & from, const Vector2 & end_pos);
public:
    Sequencer(const SequenceLimits & limits, const SequenceParas & paras):
        limits_(limits), paras_(paras){}
    
    //圆弧插补 未实现
    void arc(Curve & curve, const Ray2_t<real_t> & from, const Ray2_t<real_t> & to, const real_t & radius);
    
    //侧向移动
    void sideways(Curve & curve, const Ray2_t<real_t> & from, const Ray2_t<real_t> & to);

    //径向移动
    void follow(Curve & curve, const Ray2_t<real_t> & from, const Ray2_t<real_t> & to);

    //平移
    void shift(Curve & curve, const Ray2_t<real_t> & from, const Ray2_t<real_t> & to);

    //旋转
    void spin(Curve & curve, const Ray2_t<real_t> & from, const Ray2_t<real_t> & to);

    void wait(Curve & curve, const Ray2_t<real_t> & from, const real_t & dur);
};
}