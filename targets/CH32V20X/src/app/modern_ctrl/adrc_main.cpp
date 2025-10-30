#include "src/testbench/tb.h"
#include "dsp/controller/adrc/leso.hpp"
#include "dsp/controller/adrc/command_shaper.hpp"

#include "robots/repl/repl_service.hpp"
#include "robots/mock/mock_burshed_motor.hpp"

#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "hal/analog/adc/adcs/adc1.hpp"
#include "hal/bus/uart/uartsw.hpp"

#include "core/clock/time.hpp"


using namespace ymd;

static constexpr auto UART_BAUD = 576000;


template<size_t Q>
static constexpr fixed_t<Q, int32_t> sat(const fixed_t<Q, int32_t> x){
    // constexpr int32_t MASK = (~((1 << Q) - 1)) & (0x7fffffff);
    // const auto x_i32 = x.as_i32();
    if(ABS(x) > 1){
        return sign(x);
        // constexpr size_t SHIFT_BITS = 32 - Q;
        // return fixed_t<Q, int32_t>::from_i32(((x_i32 & (0x80000000)) >> SHIFT_BITS) | (1 << Q));
    }else{
        // return fixed_t<Q, int32_t>::from_i32(x_i32);
        return x;
    }
}


constexpr int32_t MASK = (~((1 << 16) - 1)) & (0x7fffffff);


static constexpr inline q16 fhan(
    const q16 v, 
    const q16 z1, 
    const q16 z2, 
    const q10 r,
    const q10 h
){
    const q10 d = r * h;//const
    const q10 d0 = d * h;//const
    const q16 y = z1 - v + z2 * h;//var
    const q16 abs_y = ABS(y);
    const q16 a0 = sqrt(square(q13(d)) + q13(8 * r * abs_y));//var
    const auto a = [&]{
        if(abs_y > d0){
            return z2 + ((a0 - d) >> 1) * sign(y);//var
        }else{
            return z2 + y /h;//var
        }
    }();

    return [&]{
        if(ABS(a) > d){
            return  -r * sign(a);//var
        }else{
            return -r * (a /d);//var
        }
    }();
} 


struct FhanPrecomputed{
    struct Config{
        q10 r;
        q10 h;
    };

    constexpr explicit FhanPrecomputed(const Config & cfg):
        r_(cfg.r),
        h_(cfg.h),
        d_(cfg.r * cfg.h),
        d0_(q10(cfg.r * cfg.h) * cfg.h),
        inv_h_(1 / cfg.h),
        inv_d_(1 / q10(cfg.r * cfg.h)){;}

    [[nodiscard]] constexpr q16 operator()(
        const q16 v, 
        const q16 z1, 
        const q16 z2
    ) const{
        // return fhan(v, z1, z2, r_, h_);

        // const auto r = r_;
        // const auto h = h_;
        // const q10 d = r_ * h_;//const
        // const q10 d0 = d * h_;//const
        const q16 y = z1 - v + z2 * h_;//var
        const q16 abs_y = ABS(y);
        const q16 a0 = sqrt(square(q13(d_)) + q13(8 * r_ * abs_y));//var
        const auto a = [&]{
            if(abs_y > d0_){
                return z2 + ((a0 - d_) >> 1) * sign(y);//var
            }else{
                return z2 + y * inv_h_;//var
            }
        }();

        return [&]{
            // return  -r_ * sign(a);//var
            // return -r_ * (a * inv_d_);//var
            if(ABS(a) > d_){
                return  -r_ * sign(a);//var
            }else{
                return -r_ * (a * inv_d_);//var
            }
        }();

    }
private:
    q10 r_;
    q10 h_;
    q10 d_;
    q10 d0_;
    q16 inv_h_;
    q16 inv_d_;
};

#if 0

struct NonlinearTrackingDifferentor{
    struct Config{
        uint32_t fs;
        q16 r;
        q16 h;
    };

    constexpr explicit NonlinearTrackingDifferentor(const Config & cfg):
        dt_(1_q24 / cfg.fs), r_(cfg.r), h_(cfg.h){;}
    

    constexpr void update(const q16 v){
        const auto u = fhan(v, z_[0], z_[1], r_, h_);
        const auto next_z1 = z_[0] + dt_ * z_[1];
        const auto next_z2 = z_[1] + dt_ * u;
        z_ = std::array{next_z1, next_z2};
    }

    constexpr std::array<q16, 2> state(){
        return {z_[0], z_[1]};
    }
private:
    q24 dt_;
    q16 r_;
    q16 h_;
    std::array<q16, 2> z_ = {0, 0};
};
#else
struct NonlinearTrackingDifferentor{
    struct Config{
        uint32_t fs;
        q16 r;
        q16 h;
    };

    constexpr explicit NonlinearTrackingDifferentor(const Config & cfg):
        dt_(1_q24 / cfg.fs), 
        fhan_(FhanPrecomputed(FhanPrecomputed::Config{.r = cfg.r, .h = cfg.h}))
        {;}
    

    constexpr void update(const q16 v){
        const auto u = fhan_(v, z_[0], z_[1]);
        const auto next_z1 = z_[0] + dt_ * z_[1];
        const auto next_z2 = z_[1] + dt_ * u;
        z_ = std::array{next_z1, next_z2};
    }

    constexpr std::array<q16, 2> state(){
        return {z_[0], z_[1]};
    }
private:
    q24 dt_;
    FhanPrecomputed fhan_;
    std::array<q16, 2> z_ = {0, 0};
};
#endif


void adrc_main(){
    auto init_debugger = []{
        auto & DBG_UART = DEBUGGER_INST;

        DBG_UART.init({
            .baudrate = UART_BAUD
        });

        DEBUGGER.retarget(&DBG_UART);
        DEBUGGER.set_eps(4);
        DEBUGGER.set_splitter(",");
        DEBUGGER.no_brackets(EN);
    };

    init_debugger();

    // const auto tau = 80.0_r;

    // static dsp::Leso leso{dsp::Leso::Config{
    //     .b0 = 1,
    //     .w = 17.8_r,
    //     .fs = 1000
    // }};

    // static dsp::CommandShaper1 cs{{
    //     .kp = tau * tau,
    //     .kd = 2 * tau,
    //     .max_spd = 40.0_r,
    //     // .max_acc = 200.0_r,
    //     // .max_acc = 80.0_r,
    //     .max_acc = 100.0_r,
    //     .fs = 1000
    // }};

    static NonlinearTrackingDifferentor cs{
        typename NonlinearTrackingDifferentor::Config{
        .fs = 20000,
        // .r = 30.5_q24,
        // .h = 2.5_q24
        .r = 252.5_q10,
        .h = 0.012_q10
    }};

    q16 u = 0;
    Microseconds elapsed_micros = 0us;


    [[maybe_unused]]
    auto command_shaper_poller = [&](){

        const auto u0 = clock::micros();
        cs.update(u);
        const auto u1 = clock::micros();
        elapsed_micros = u1 - u0;

        // leso.update(cs.state()[0], u);

    };

    auto & timer = hal::timer1;
    timer.init({
        .count_freq = hal::NearestFreq(20000),
        .count_mode = hal::TimerCountMode::Up
    }, EN);


    timer.register_nvic<hal::TimerIT::Update>({0,0}, EN);
    timer.enable_interrupt<hal::TimerIT::Update>(EN);
    timer.set_event_callback([&](hal::TimerEvent ev){
        switch(ev){
        case hal::TimerEvent::Update:{
            command_shaper_poller();
            break;
        }
        default: break;
        }
    });

    while(true){
        const auto ctime = clock::time();
        u = 10 * sign(sinpu(ctime * 0.5_r));

        DEBUG_PRINTLN(
            u,
            cs.state()[0],
            cs.state()[1],
            sat(cs.state()[0])
            // leso.get_disturbance()
            ,elapsed_micros.count()
        );

        clock::delay(2ms);
    }

}