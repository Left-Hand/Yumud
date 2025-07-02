#pragma once

// #include "dsp/constexprmath/ConstexprMath.hpp"
#include "core/math/realmath.hpp"
namespace ymd::dsp{


template<typename T>
class SecondOrderTransferFunc{
    struct Coefficient{
        T a1, a2, b0, b1, b2;
    };

    T b0_, b1_, b2_;  // 分子系数
    T a1_, a2_;       // 分母系数（通常 a0 = 1）
    T x1_, x2_;       // 输入的延迟值（x[n-1], x[n-2]）
    T y1_, y2_;       // 输出的延迟值（y[n-1], y[n-2]）
public:
    SecondOrderTransferFunc(const Coefficient & coff){
        reconf(coff);
    }

    void reconf(const Coefficient & coff){
        b0_ = coff.b0;
        b1_ = coff.b1;
        b2_ = coff.b2;
        a1_ = coff.a1;
        a2_ = coff.a2;
    }

    void reset(){
        x1_ = x2_ = y1_ = y2_ = 0;
    }

    void update(const T input){
        T output = b0_*input + b1_*x1_ + b2_*x2_ - a1_*y1_ - a2_*y2_;
    
        // 更新状态变量
        x2_ = x1_;
        x1_ = input;
        y2_ = y1_;
        y1_ = output;
        
        // return output;
    }

    T result() const {
        return y1_;
    }
    
    struct BpfConfig{
        T fl;
        T fh;
        uint fs;
    };

    static constexpr 
    Coefficient make_butterworth_bpf(const BpfConfig & config){
        const T fl = config.fl;    // 输入下限频率（Hz）
        const T fh = config.fh;    // 输入上限频率（Hz）
        const auto fs = config.fs;    // 采样率（Hz）
    
        // 1. 归一化到奈奎斯特频率（fs/2）
        const T fl_norm = fl / (fs / 2); 
        const T fh_norm = fh / (fs / 2);
    
        // 2. 计算中心频率和带宽（归一化到 [0, 1]）
        // const T omega0 = (fl_norm + fh_norm) / 2;      // 中心频率（几何平均可替换）
        const T omega0 = std::sqrt(fl_norm * fh_norm);
        const T bandwidth = fh_norm - fl_norm;         // 带宽
    
        // 3. 转换为弧度（单位：rad/sample）
        const T omega0_rad = omega0 * T(PI); 
        const T bandwidth_rad = bandwidth * T(PI);
    
        // 4. 计算参数 α（核心公式）
        const T sin_half_band = std::sin(bandwidth_rad / 2);
        const T cos_omega = std::cos(omega0_rad - bandwidth_rad / 2);
        const T alpha = sin_half_band / (2 * cos_omega);
    
    
        // // 返回系数结构体（注意分母系数 a1, a2 的符号）
        return Coefficient{
            .a1 = -2 * std::cos(omega0_rad),
            .a2 = 1 - 2 * alpha,
            .b0 = alpha,
            .b1 = 0,
            .b2 = -alpha,
        };
    }
};

}