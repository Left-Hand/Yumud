#pragma once

#include "core/platform.hpp"

namespace ymd::robots{

// https://zhuanlan.zhihu.com/p/652803608

//对当前量与目标量进行梯形位置求解
template<arithmetic T>
class SigmoidSolver_t final{
public:
    DELETE_COPY_AND_MOVE(SigmoidSolver_t);

    SigmoidSolver_t(const T & a, const T & v, const T & s):
        s_(s) {
        //TODO calculate max t
    }

    T forward(const T t) const{
        if(t > t_all) return s_;
        return s(t / t_all) * s_;
    }

    T period() const {
        return t_all;
    }

private:
    T s_;

    T t_all;
    bool peaked;
    
    __inline T s(const T & x){
        static constexpr double k = 1.004969823;
        static constexpr double m = (1.0 - k) / 2;
        return T(k) * (1/(exp(-(12 * x - 6)) + 1)) + T(b);
    }
};


}