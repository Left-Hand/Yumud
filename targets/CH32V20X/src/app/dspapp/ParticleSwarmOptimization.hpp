#pragma once
// #pragma GCC optimize(3,"Ofast","inline")

#include <cstdlib>
#include <span>
#include "core/utils/hive.hpp"

#include "core/math/real.hpp"
#include "core/debug/debug.hpp"
#include "dsp/siggen/noise/LCGNoiseSiggen.hpp"
#include <random>

namespace ymd::dsp{

template<typename D, arithmetic T = real_t>
struct Particle{
    D x;
    D v;
    T pbest;
    T peval;

    void reset(){
        x = D{0};
        v = D{0};
        pbest = std::numeric_limits<T>::lowest();
        peval = std::numeric_limits<T>::lowest();
    }
};


template<typename P, arithmetic T = real_t>
class ParticleSwarmOptimization{
public:
    // using EvalFunc = std::function<T(const P &)>;

    struct Config{
        T omega;
        T c1;
        T c2;
        uint n;
    };

    ParticleSwarmOptimization(const Config & cfg){
        reconf(cfg);
        reset();
        // init();
    }

    void init(T from, T to){
        for(auto & p: particles_){
            // p.reset();
            p.x = from + rand() * (to - from);
        }
    }

    void reconf(const Config & cfg){
        omega_ = cfg.omega;
        c1_ = cfg.c1;
        c2_ = cfg.c2;
        // particles_.reserve(cfg.n);
        particles_.resize(cfg.n);
        reset();
        // eval_func_ = std::move(cfg.eval_func);
        // eval_func_ = cfg.eval_func;
        // reset_func_ = std::move(cfg.reset_func);
    }

    void reset(){
        for(auto & p: particles_){
            p.reset();
        }
        best_idx_.reset();
    }

    void retarg(){
        gbest_ = std::numeric_limits<T>::lowest();
        geval_ = std::numeric_limits<T>::lowest();
    }
    
    template<typename Fn>
    void update(Fn && eval_func_){

        if(best_idx_.has_value()) {
            const auto temp_geval_ = std::forward<Fn>(eval_func_)(
                particles_[best_idx_.value()]);
            geval_ = temp_geval_;
        }

        for(size_t i = 0; i < particles_.size(); ++i){
            auto & p = particles_[i];
            const auto temp_p_eval = std::forward<Fn>(eval_func_)(p);
            
            if((temp_p_eval > p.peval)){
                p.peval = temp_p_eval;
                p.pbest = p.x;
            }

            if((p.peval > geval_)){
                geval_ = p.peval;
                gbest_ = p.pbest;

                best_idx_ = i;
            }

            // const auto r1 = rand();
            // const auto r2 = rand();
            const auto [r1,r2] = rand2();
            // const auto r2 = r1;

            p.v = 
                + (omega_ * p.v) 
                + (2 * r1 * (p.pbest - p.x)) 
                + (2 * r2 * (gbest_ - p.x));
            p.x = p.x + p.v;
        }


    }

    const auto & particles() const{
        // return std::span(particles_);
        return particles_;
    }

    T gbest() const{
        return gbest_;
    }

    T geval() const{
        return geval_;
    }

    auto bestidx() const{
        return best_idx_;
    }

private:

    __fast_inline iq_t<16> rand(){
        // return CLAMP(T(randint(65536)) / 65536, 0, 1);
        // return CLAMP(T(randint(65535)) / 65536, 0, 1);
        // return frac(T(randint(65535)) / 65536);
        // const auto ret = iq_t<16>(std::bit_cast<_iq<16>>(int32_t((lcg_.update() >> 16) & 0xffff)));
        const auto ret = iq_t<16>(std::bit_cast<_iq<16>>(int32_t((lcg_.update() >> 16))));
        // DEBUG_PRINTLN(ret);
        // delay(1);
        return ret;
        // return 1;
        // return rng_.
    }

    __fast_inline std::tuple<iq_t<16>, iq_t<16>> rand2(){

        const auto ra = (lcg_.update());
        const auto ret = iq_t<16>(std::bit_cast<_iq<16>>(uint32_t(ra >> 16)));
        const auto ret2 = iq_t<16>(std::bit_cast<_iq<16>>(uint32_t(uint16_t(ra))));
        return {ret,ret2};
    }


    std::vector<P> particles_ = {};
    T c1_ = {};
    T c2_ = {};

    T omega_ = {};
    std::optional<size_t> best_idx_ = std::nullopt;
    T gbest_ = std::numeric_limits<T>::lowest();
    T geval_ = std::numeric_limits<T>::lowest();

    LCGNoiseSiggen lcg_;
};

}