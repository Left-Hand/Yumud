#pragma once

namespace ymd::dsp{
//Z域低通滤波器
template<arithmetic T>
struct LowpassFilter_t{
public: 
    struct Config{
        T fc;
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

    [[nodiscard] ]constexpr const T & get() const {
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

}