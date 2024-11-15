#pragma once

#include "memory.h"

#include "sys/math/real.hpp"
#include "sys/stream/ostream.hpp"

namespace ymd{

namespace MatirxUtils{

    template<typename T>
    __fast_inline constexpr T det(const T * _mat, int n){
        static_assert(n > 0);
        T sum = 0;
        int i = 0;
        if (n == 1){
            sum = _mat[0];
        }else if (n == 2){
            auto mat = reinterpret_cast<const T(*)[2]>(_mat);
            sum = mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];//杀戮法求解
        }else if (n == 3){
            auto mat = reinterpret_cast<const T(*)[3]>(_mat);
            sum = mat[0][0] * mat[1][1] * mat[2][2]
                + mat[0][1] * mat[1][2] * mat[2][0]
                + mat[1][0] * mat[2][1] * mat[0][2]
                - mat[0][2] * mat[1][1] * mat[2][0]
                - mat[0][1] * mat[1][0] * mat[2][2]
                - mat[1][2] * mat[2][1] * mat[0][0];//划线法求解
        }
        // }else{
        //     for (i = 0; i < n; i++){
        //         if (mat[0][i] != 0){
        //             sum += ((int)pow(-1, i + 0)) * mat[0][i] * (cof(mat, i, n));//2阶以上继续递归		
        //         }
        //         else
        //             sum += 0;//展开项为0
        //     }
        // }
        return sum;
    }
}

}