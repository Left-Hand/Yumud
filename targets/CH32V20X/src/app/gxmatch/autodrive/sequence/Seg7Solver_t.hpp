#pragma once

#include "sys/core/platform.h"

//对当前量与目标量进行7段S曲线位置求解
//未实现

namespace gxm{


template<arithmetic T>
class Seg7Solver_t{
protected:
    T j_;
    T a_;
    T v_;
    T s_;

    // T t1;
    // T t2;
    // T t_all;
    // T s1;
    // bool peaked;
    
public:
    DELETE_COPY_AND_MOVE(Seg7Solver_t);
    Seg7Solver_t(T j, T a, T v, T s): j_(j), a_(a), v_(v), s_(s){
        // t1 = 0;
        // t2 = 0;
        // t_all = 0;
        // s1 = 0;
        // peaked = false;
    }
    
};

}