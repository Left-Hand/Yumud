#pragma once

#include "SequenceUtils.hpp"
#include "SequencerBase.hpp"

namespace gxm{

//路径序列化器 将几何路径渲染为位姿集
class Sequencer:public SequencerBase{
protected:
    const SequenceLimits & limits_;
    const SequenceParas & paras_;

    void rotate(Curve & curve, const Ray & from, const real_t & end_rad);

    void linear(Curve & curve, const Ray & from, const Vector2 & end_pos);
public:
    Sequencer(const SequenceLimits & limits, const SequenceParas & paras):
        limits_(limits), paras_(paras){}
    
    //圆弧插补 未实现
    void arc(Curve & curve, const Ray & from, const Ray & to, const real_t & radius);
    
    //侧向移动
    void sideways(Curve & curve, const Ray & from, const Ray & to);

    //径向移动
    void follow(Curve & curve, const Ray & from, const Ray & to);

    //平移
    void shift(Curve & curve, const Ray & from, const Ray & to);

    //旋转
    void spin(Curve & curve, const Ray & from, const Ray & to);
};
}