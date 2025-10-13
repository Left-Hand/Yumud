#pragma once

#include "core/platform.hpp"

//对当前量与目标量进行7段S曲线位置求解
//未实现

namespace ymd::robots{


template<arithmetic T>
struct Seg7Solver final{
public:
    struct Config{
        T max_jerk;
        T max_acc;
        T max_speed;
    };
    Seg7Solver(const Seg7Solver & other) = delete;
    Seg7Solver(Seg7Solver && other) = delete;
    Seg7Solver(T j, T a, T v, T s): j_(j), a_(a), v_(v), s_(s){
        // t1 = 0;
        // t2 = 0;
        // t_all = 0;
        // s1 = 0;
        // peaked = false;
    }
private:
    T j_;
    T a_;
    T v_;
    T s_;

    // T t1;
    // T t2;
    // T t_all;
    // T s1;
    // bool peaked;
};

}