#pragma once
#include <cstdint>

// 可实现小数群超前的全通滤波器

namespace ymd::dsp {


template<typename T>
class SubSampleLeadCompensator {
public:
    struct Config {
        T lead_time;  // 超前时间（采样周期为单位）
        T beta;       // 预测滤波器系数（0.5~0.9）
    };
    
    void reconf(const Config& cfg) {
        lead_time_ = cfg.lead_time;
        beta_ = cfg.beta;
        
        // 分离整数和分数部分
        T int_part = std::floor(lead_time_);
        T frac_part = lead_time_ - int_part;
        
        // 整数超前通过预测实现
        prediction_steps_ = static_cast<int>(int_part);
        
        // 分数部分使用全通补偿
        if (frac_part > T(0.001)) {
            alpha_ = (T(1) - frac_part) / (T(1) + frac_part);
        } else {
            alpha_ = T(0);
        }
    }
    
    T operator()(T in) {
        // 更新历史缓冲区
        history_[current_idx_] = in;
        current_idx_ = (current_idx_ + 1) % BUFFER_SIZE;
        
        // 线性预测
        T predicted = predict();
        
        // 应用分数超前补偿
        return apply_fractional_lead(predicted);
    }
    
private:
    static constexpr int BUFFER_SIZE = 10;
    T history_[BUFFER_SIZE] = {0};
    int current_idx_ = 0;
    T alpha_ = 0;
    T lead_time_ = 0;
    T beta_ = T(0.7);
    int prediction_steps_ = 0;
    
    T predict() {
        if (prediction_steps_ <= 0) return history_[(current_idx_ - 1) % BUFFER_SIZE];
        
        // 简单线性外推：y[n+k] ≈ y[n] + k*(y[n]-y[n-1])
        T y0 = history_[(current_idx_ - 1) % BUFFER_SIZE];
        T y1 = history_[(current_idx_ - 2) % BUFFER_SIZE];
        T derivative = beta_ * (y0 - y1);
        
        return y0 + prediction_steps_ * derivative;
    }
    
    T apply_fractional_lead(T in) {
        static T latch = 0;
        T v = latch;
        T t = in - alpha_ * v;
        latch = t;
        return v + alpha_ * t;
    }
};

}