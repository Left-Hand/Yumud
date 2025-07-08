#pragma once

#include "core/math/real.hpp"
#include "core/stream/ostream.hpp"
#include "core/math/realmath.hpp"

#include "digipw/prelude/abdq.hpp"
#include "dsp/filter/butterworth/ButterBandFilter.hpp"
#include "dsp/filter/butterworth/ButterSideFilter.hpp"


namespace ymd::foc{



static __inline real_t sign_sqrt(const real_t x){
    return x < 0 ? -sqrt(-x) : sqrt(x);
};

static __inline real_t smooth(const real_t x){
    return x - sin(x);
}



}

namespace ymd{


struct CurrentSensor{

    static constexpr real_t CURRENT_CUTOFF_FREQ = 400;

    struct Config{
        uint32_t fs;
    };

    constexpr explicit CurrentSensor(const Config & cfg):
        mid_filter_({CURRENT_CUTOFF_FREQ, cfg.fs}){
    }

    constexpr void reconf(const Config & cfg){
        mid_filter_.reconf({CURRENT_CUTOFF_FREQ, cfg.fs});
    }



    constexpr void update(const real_t u, const real_t v, const real_t w){
        uvw_raw_ = {u,v,w};

        mid_filter_.update((uvw_raw_.u + uvw_raw_.v + uvw_raw_.w) * q16(1.0/3));

        mid_curr_ = mid_filter_.get();
        uvw_curr_[0] = (uvw_raw_.u - mid_curr_ - uvw_bias_.u);
        uvw_curr_[1] = (uvw_raw_.v - mid_curr_ - uvw_bias_.v);
        uvw_curr_[2] = (uvw_raw_.w - mid_curr_ - uvw_bias_.w);

        ab_curr_ = digipw::AbCurrent::from_uvw(uvw_curr_);
    }

    constexpr void reset(){
        uvw_curr_ = {0, 0, 0};
        uvw_bias_ = {0, 0, 0};
        ab_curr_ = {0, 0};
    }


    constexpr const auto &  raw()const {return uvw_raw_;}
    constexpr const auto &  mid() const {return mid_curr_;}
    constexpr const auto &  uvw()const{return uvw_curr_;}
    constexpr const auto & ab()const{return ab_curr_;}

private:

    digipw::UvwCurrent uvw_bias_;
    digipw::UvwCurrent uvw_raw_;
    digipw::UvwCurrent uvw_curr_;
    real_t mid_curr_;
    digipw::AbCurrent ab_curr_;
    dsp::ButterLowpassFilter<q16, 4> mid_filter_;
};



template<typename From, typename Protocol = void>
struct Serializer{};

template<typename From, typename Protocol = void>
struct Deserializer{};


template<typename T, typename Protocol = void>
static constexpr auto serialize(T && obj){
    return Serializer<std::decay_t<T>, Protocol>::serialize(std::forward<T>(obj));
}


enum class DeserializeError:uint8_t{
    BytesLengthShort
};


template<typename T, typename Protocol = void>
static constexpr Result<T, DeserializeError> deserialize(std::span<const uint8_t> data) {
    return Deserializer<T, Protocol>::deserialize(data);
}

template<size_t N>
struct Serializer<iq_t<N>>{
    static constexpr std::array<uint8_t, 4> serialize(const iq_t<N> num){
        const auto inum = num.to_i32();
        return std::bit_cast<std::array<uint8_t, 4>>(inum);
    } 
};

template<size_t N>
struct Deserializer<iq_t<N>> {
    static constexpr Result<iq_t<N>, DeserializeError> 
    deserialize(std::span<const uint8_t> pbuf) {
        if(pbuf.size() < 4) return Err(DeserializeError::BytesLengthShort);
        int32_t val = std::bit_cast<int32_t>(
            std::array<uint8_t, 4>{pbuf[0], pbuf[1], pbuf[2], pbuf[3]});
        return Ok(iq_t<N>::from_i32(val));
    }
};

template<typename T>
requires std::is_integral_v<T>
struct Serializer<T> {
    static constexpr size_t N = sizeof(T);
    static constexpr std::array<uint8_t, N> serialize(const T inum) {
        return std::bit_cast<std::array<uint8_t, N>>(inum);
    }
};

template<typename T>
requires std::is_integral_v<T>
struct Deserializer<T> {
    static constexpr Result<T, DeserializeError> 
    deserialize(std::span<const uint8_t> data) {
        if (data.size() < sizeof(T)) {
            return Err(DeserializeError::BytesLengthShort);
        }

        std::array<uint8_t, sizeof(T)> bytes{};
        std::copy_n(data.data(), sizeof(T), bytes.begin());
        return Ok(std::bit_cast<T>(bytes));
    }
};

template<typename T>
requires std::is_floating_point_v<T>
struct Deserializer<T> {
    static constexpr Result<T, DeserializeError> 
    deserialize(std::span<const uint8_t> data) {
        if (data.size() < sizeof(T)) {
            return Err(DeserializeError::BytesLengthShort);
        }

        std::array<uint8_t, sizeof(T)> bytes{};
        std::copy_n(data.data(), sizeof(T), bytes.begin());
        return Ok(std::bit_cast<T>(bytes));
    }
};

}


namespace ymd::dsp{
template<typename T>
struct ComplementaryFilter{
    struct Config{
        T kq;
        T ko;
        uint fs;
    };
    

    constexpr ComplementaryFilter(const Config & config){
        reconf(config);
        reset();
    }


    constexpr void reconf(const Config & cfg){
        kq_ = cfg.kq;
        kq_ = cfg.kq;
        dt_ = T(1) / cfg.fs;
    }

    constexpr T operator ()(const T rot, const T gyr){

        if(!inited_){
            rot_ = rot;
            rot_unfiltered_ = rot;
            inited_ = true;
        }else{
            rot_unfiltered_ += gyr * delta_t_;
            rot_unfiltered_ = kq_ * rot_ + (1-kq_) * rot;
            rot_ = ko_ * rot_ + (1-ko_) * rot_unfiltered_;
        }
    
        last_rot_ = rot;
        last_gyr_ = gyr;

        return rot_;
    }

    constexpr void reset(){
        rot_ = 0;
        rot_unfiltered_ = 0;
        last_rot_ = 0;
        last_gyr_ = 0;
        inited_ = false;
    }

    constexpr T get() const {
        return rot_;
    }

private:
    T kq_;
    T ko_;
    T dt_;
    T rot_;
    T rot_unfiltered_;
    T last_rot_;
    T last_gyr_;
    // T last_time;

    uint delta_t_;
    
    bool inited_;
};

}


// struct TurnSolver{
//     uint16_t ta = 0;
//     uint16_t tb = 0;
//     real_t pa = 0;
//     real_t pb = 0;
//     real_t va = 0;
//     real_t vb = 0;
// };

// [[maybe_unused]] static real_t demo(uint milliseconds){
//     // using Vector2<real_t> = CubicInterpolation::Vector2<real_t>;
//     static TurnSolver turnSolver;
    
//     uint32_t turnCnt = milliseconds % 2667;
//     uint32_t turns = milliseconds / 2667;
    
//     scexpr real_t velPoints[7] = {
//         real_t(20)/360, real_t(20)/360, real_t(62.4)/360, real_t(62.4)/360, real_t(20.0)/360, real_t(20.0)/360, real_t(20.0)/360
//     };
    
//     scexpr real_t posPoints[7] = {
//         real_t(1.0f)/360,real_t(106.1f)/360,real_t(108.1f)/360, real_t(126.65f)/360, real_t(233.35f)/360,real_t(359.0f)/360,real_t(361.0f)/360
//     };

//     scexpr uint tickPoints[7] = {
//         0, 300, 400, 500, 2210, 2567, 2667 
//     };

//     int8_t i = 6;

//     while((turnCnt < tickPoints[i]) && (i > -1))
//         i--;
    
//     turnSolver.ta = tickPoints[i];
//     turnSolver.tb = tickPoints[i + 1];
//     auto dt = turnSolver.tb - turnSolver.ta;

//     turnSolver.va = velPoints[i];
//     turnSolver.vb = velPoints[i + 1];
    
//     turnSolver.pa = posPoints[i];
//     turnSolver.pb = posPoints[i + 1];
//     real_t dp = turnSolver.pb - turnSolver.pa;

//     real_t _t = ((real_t)(turnCnt  - turnSolver.ta) / dt);
//     real_t temp = (real_t)dt / 1000 / dp; 

//     real_t yt = 0;

//     if((i == 0) || (i == 2) || (i == 4))
//         yt = CubicInterpolation::forward(
//             Vector2<real_t>{real_t(0.4f), real_t(0.4f) * turnSolver.va * temp}, 
//             Vector2<real_t>(real_t(0.6f), real_t(1.0f) - real_t(0.4f)  * turnSolver.vb * temp), _t);
//     else
//         yt = _t;

//     real_t new_pos =  real_t(turns) + turnSolver.pa + dp * yt;

//     return new_pos;
// }
