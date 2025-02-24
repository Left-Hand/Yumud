#pragma once

#include "../lti.hpp"

//Z域低通滤波器
template<arithmetic T>
struct LowpassFilter_t{
public: 
    struct Config{
        uint fc;
        uint fs;
    };

private:
    T alpha_;
    T last_;
    bool inited_;
public:
    constexpr LowpassFilter_t(){
        reset();
    }

    constexpr LowpassFilter_t(const Config & config){
        reconf(config);
        reset();
    }

    constexpr void reconf(const Config & config){
        alpha_ = solve_alpha(config.fc, config.fs);
    }

    constexpr void reset(){
        last_ = 0;
        inited_ = false;
    }

    constexpr void update(const T x){
        if(unlikely(inited_ == false)){
            last_ = x;
            inited_ = true;
        }else{
            last_ = (last_ * (1-alpha_) + (alpha_) * x);
        }
        // return last_;
    }

    constexpr T result() const {
        return last_;
    }

    static constexpr T solve_alpha(const auto fc, const auto fs){
        // scexpr uint scaler = 100;
        // scexpr uint tau_sal = uint(TAU * scaler);
        // return T(tau_sal) * fc / (scaler * fs + tau_sal * fc);

        return T(TAU) * fc / (fs + T(TAU) * fc);
    }
};

template<arithmetic T>
struct LowpassFilterD_t{
public:
    struct Config {
        uint fc;
        uint fs;
    };
private:
    T alpha_;
    T wc_;

    T last_;
    T last_x_;
    bool inited_;
public:
    constexpr LowpassFilterD_t(){
        reset();
    }

    constexpr LowpassFilterD_t(const Config & config){
        reconf(config);
        reset();
    }

    constexpr void reconf(const Config & config){
        alpha_ = solve_alpha(config.fc, config.fs);
        wc_ = T(TAU) * config.fc;
    }

    constexpr void reset(){
        last_ = 0;
        last_x_ = 0;
        inited_ = false;
    }

    constexpr void update(const T x){
        if(unlikely(inited_ == false)){
            last_ = x;
            last_x_ = x;
            inited_ = true;
        }else{
            last_ = alpha_ * (last_ + x - last_x_);
            last_x_ = x;
        }

        // return last_ * wc_;
    }

    constexpr T result() const {
        return last_ * wc_;
    }

    static constexpr T solve_alpha(const auto fc, const auto fs){
        return (T(fs) / (fs + T(TAU) * fc));

    }
};

template <arithmetic T>
class SecondOrderLowpassFilter_t {

public:
    struct Config {
        uint32_t fc;  // Cutoff frequency in Hz
        uint32_t fs;  // Sampling frequency in Hz
    };

private:
    T a0_, a1_, a2_, a3_, a4_;
    T x1, x2, y1, y2;
    bool inited_;

public:
    constexpr SecondOrderLowpassFilter_t(){
        reset();
    }

    constexpr SecondOrderLowpassFilter_t(const Config &config) {
        reconf(config);
        reset();
    }

    constexpr void reconf(const Config &config) {
        scexpr T q_butterworth = T(0.7071);

        T b0, b1, b2, a0, a1, a2;
     
        const T w0 = T(TAU) * config.fc / config.fs;
        const T sin_w0 = T(sin((w0)));
        const T cos_w0 = T(cos((w0)));
        const T alpha = sin_w0 / (2 * q_butterworth);
     

        b0 = (1 - cos_w0) / 2;
        b1 = b0 * 2;
        b2 = b0;
        a0 = 1 + alpha;
        a1 = -2 * cos_w0;
        a2 = 1 - alpha;

        a0_ = b0 / a0;
        a1_ = b1 / a0;
        a2_ = b2 / a0;
        a3_ = a1 / a0;
        a4_ = a2 / a0;
    }

    constexpr void reset() {
        x1 = 0;
        y1 = 0;
        x2 = 0;
        y2 = 0;
        inited_ = false;
    }

    constexpr void update(const auto _x) {
        const T x = T(_x);
        if (unlikely(!inited_)) {
            x1 = x;
            y1 = x;
            x2 = x;
            y2 = x;
            inited_ = true;
        } else {
            // Compute the new output using the difference equation
            const T result = a0_ * x + a1_ * x1 + a2_ * x2 -  a3_ * y1 - a4_ * y2;
            x2 = x1;
            x1 = x;
            y2 = y1;
            y1 = result;
        }
    }

    constexpr T result() const {
        return y1;
    }
};