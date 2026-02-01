
#include "src/testbench/tb.h"
#include "core/clock/time.hpp"
#include "core/debug/debug.hpp"

#include "hal/timer/hw_singleton.hpp"

#include "dsp/filter/butterworth/Order4ZeroPhaseShiftButterWothLowpassFilter.hpp"
#include "dsp/filter/firstorder/lpf.hpp"

#include "ParticleSwarmOptimization.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/uart/hw_singleton.hpp"

using namespace ymd;

using Particle = dsp::Particle<real_t, real_t>;

void pso_tb() __attribute((optimize(3,"Ofast","inline")));

void pso_tb(){
    using Pso = dsp::ParticleSwarmOptimization<Particle, real_t>;
    using Config = typename Pso::Config;


    // auto eval_func = [&](const Particle & p){
    //     return -ABS(p.x - 5.5_r + sin(t));
    //     // return -ABS(p.x - 0.5_r);
    // };

    Pso pso = {Config{
        .omega = 0.2_r,
        .c1 = 2.0_r,
        .c2 = 2.0_r,
        // .c1 = 2.0_r,
        // .c2 = 2.0_r,
        // .c1 = 1.3_r,
        // .c2 = 1.3_r,
        .n = 8,
        // .eval_func = eval_func
        
        // .reset_func = nullptr;
    }};

    pso.init(-10, 1);
    auto eval_func = [](const Particle & p){
        // return -ABS(p.x - 5.5_r + sin(clock::seconds()));
        // return -ABS(p.x - 0.5_r);
        // const auto targ = sin(2 * time());
        // DEBUG_PRINTLN(targ);
        // return -ABS(p.x - 102.5_r);
    
        // if (ABS(p.x) <= 0.02_r) return 1;
        // return sin(p.x) / p.x;
    
        const auto x = p.x;
        return x * (2-x);
        // return -ABS(x);
        // return sin(x);
        // return CLAMP(1200 * cos(x) + (120-x) * x, 0, 10000);
        // return p.x;
        // return 0;
    };
    

    constexpr size_t loops = 200;

    const auto begin_ms = clock::micros();


    for(size_t i = 0; i < loops; i++){

        pso.update(eval_func);

        // const auto & particles = pso.particles();
        // for(const auto & p : particles){
            // DEBUG_PRINT(p.x);
            // DEBUG_PRINT(DEBUGGER.splitter());
            // clock::delay(1ms);
        // }
        // clock::delay(1ms);
        // DEBUG_PRINTLN(pso.gbest(), pso.geval());
    }
    DEBUG_PRINTLN(pso.gbest(), pso.geval(), clock::micros() - begin_ms);
}