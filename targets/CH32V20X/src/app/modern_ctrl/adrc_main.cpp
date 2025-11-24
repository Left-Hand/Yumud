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


template<typename T, size_t Orders>
struct LinearTrackingDifferentiator;



template<>
struct LinearTrackingDifferentiator<iq16, 2>{
public:
    using delta_type = iq30;
    struct [[nodiscard]] Coeffs{
        delta_type dt;
        delta_type r_by_fs;
        delta_type r2_by_fs;
    };

    struct [[nodiscard]] Config{
        uint32_t fs;
        iq8 r;

        constexpr Result<Coeffs, const char *> try_to_coeffs() const {
            auto & self = *this;
            if(self.r * self.r > self.fs)
                return Err("r^2 > fs");
            const auto dt = iq30(1) / self.fs;
            const auto r_by_fs = dt * self.r;
            const auto r2_by_fs = r_by_fs * self.r;
            return Ok(Coeffs{
                dt,
                r_by_fs,
                r2_by_fs
            });
        }
    };


    struct State{
        iq16 position;
        iq16 speed;

        constexpr void reset(){
            position = 0;
            speed = 0;
        }
    };


    constexpr explicit LinearTrackingDifferentiator(const Coeffs & coeffs):
        coeffs_(coeffs){
        reset();
    }

    constexpr void reset(){
        state_.reset();
    }

    constexpr void update(const iq16 u){

        const auto x1 = state_.position;
        const auto x2 = state_.speed;

        // 临界二阶阻尼系统
        // G(s) = 1 / (s ^ 2 + 2 * r * s + r ^ 2)
        
        state_ = State{
            x1 + x2 * coeffs_.dt,
            x2 + (iq16(- 2 * x2) * coeffs_.r_by_fs + (iq16(u - x1) * coeffs_.r2_by_fs))
        };

    }

    constexpr const State & state() const {return state_;}
private:
    State state_;
    Coeffs coeffs_;
};

constexpr int32_t MASK = (~((1 << 16) - 1)) & (0x7fffffff);


static constexpr inline iq16 fhan(
    const iq16 v, 
    const iq16 z1, 
    const iq16 z2, 
    const iq10 r,
    const iq10 h
){
    const iq10 d = r * h;//const
    const iq10 d0 = d * h;//const
    const iq16 y = z1 - v + z2 * h;//var
    const iq16 abs_y = ABS(y);
    const iq16 a0 = sqrt(square(iq13(d)) + iq13(8 * r * abs_y));//var
    const iq16 a = [&]{
        if(abs_y > d0){
            return z2 + ((a0 - d) >> 1) * sign(y);//var
        }else{
            return z2 + y /h;//var
        }
    }();

    if(ABS(a) > d){
        return  -r * sign(a);//var
    }else{
        return -r * (a /d);//var
    }

} 


template<typename T>
struct FhanPrecomputed{
    struct Config{
        iq10 r;
        iq10 h;
    };

    constexpr explicit FhanPrecomputed(const Config & cfg):
        r_(cfg.r),
        h_(cfg.h),
        d_(cfg.r * cfg.h),
        d0_(iq10(cfg.r * cfg.h) * cfg.h),
        inv_h_(1 / cfg.h),
        inv_d_(1 / iq10(cfg.r * cfg.h)){;}

    [[nodiscard]] constexpr iq16 operator()(
        const iq16 v, 
        const iq16 z1, 
        const iq16 z2
    ) const{
        const iq16 y = z1 - v + z2 * h_;//var
        const iq16 abs_y = ABS(y);
        // const iq16 a0 = sqrt(square(iq8(d_)) + iq8(8 * r_) * iq8(abs_y));//var
        const iq16 a0 = sat_sqrt(iq8(d_), iq8(8 * r_), iq8(abs_y));//var
        
        const iq16 a = [&]{
            if(abs_y > d0_){
                // return z2 + ((a0 - d_) >> 1) * sign(y);//var
                if(y > 0)
                    return z2 + ((a0 - d_) >> 1);//var
                else 
                    return z2 - ((a0 - d_) >> 1);//var
            }else{
                return z2 + y * inv_h_;//var
            }
        }();


        if(ABS(a) > d_){
            if(a > 0) 
                return  -r_;//var
            else 
                return r_;//var
        }else{
            return -r_ * (a * inv_d_);//var
        }

    }
private:
    iq10 r_;
    iq10 h_;
    iq10 d_;
    iq10 d0_;
    iq16 inv_h_;
    iq16 inv_d_;

    __fast_inline static constexpr iq16 
    sat_sqrt(const iq8 d, const iq8 _8r, const iq8 abs_y){
        int64_t sum_bits_q16 = square(static_cast<int64_t>(d.to_bits())) + 
            (static_cast<int64_t>(_8r.to_bits()) * static_cast<int64_t>(abs_y.to_bits()));
        if(sum_bits_q16 >> 32){
            // sum_bits_q16 = UINT32_MAX;
            // PANIC{};
            return iq16(256);
        }
        return sqrt(iq16::from_bits(static_cast<int32_t>(sum_bits_q16)));
    }
};



struct NonlinearTrackingDifferentor{
    using fhan_type = FhanPrecomputed<iq16>;
    struct Coeffs{
        iq30 dt;
        fhan_type fhan;
    };

    struct Config{
        uint32_t fs;
        iq16 r;
        iq16 h;

        constexpr Result<Coeffs, const char *> try_to_coeffs() const {
            const auto & self = *this;
            return Ok(Coeffs{
                .dt = (1_iq24 / fs), 
                .fhan = (fhan_type(fhan_type::Config{.r = self.r, .h = self.h}))
            });
        }
    };

    struct State{
        fixed_t<32, int64_t> z1;
        fixed_t<32, int64_t> z2;
    };

    constexpr explicit NonlinearTrackingDifferentor(const Coeffs & coeffs):
        coeffs_(coeffs)
        {;}
    

    constexpr void update(const iq16 v){
        const iq16 z1 = iq16::from_bits(static_cast<int32_t>(state_.z1.to_bits() >> 16));
        const iq16 z2 = iq16::from_bits(static_cast<int32_t>(state_.z2.to_bits() >> 16));
        
        const auto u = coeffs_.fhan(v, z1, z2);
        // const auto u = iq16(10);
        const auto next_z1 = state_.z1 + long_fixed_mul_fixed(state_.z2, coeffs_.dt);
        const auto next_z2 = CLAMP2(state_.z2 + fixed_t<32, int64_t>::from_bits(
            static_cast<int64_t>(u.to_bits()) * static_cast<int64_t>(coeffs_.dt.to_bits()) >> 14
        // ), 80);
        ), 680);
        state_ = {
            next_z1,
            next_z2
        };
    }

    constexpr const State & state() const{
        return state_;
    }
private:
    Coeffs coeffs_;
    State state_ = {0, 0};

    template<size_t Q1, size_t Q2>
    static constexpr fixed_t<Q1, int64_t> long_fixed_mul_fixed(const fixed_t<Q1, int64_t> x1, const fixed_t<Q2, int32_t> x2){
        const int64_t bits = (x1.to_bits() * int64_t(x2.to_bits())) >> Q2;
        return fixed_t<Q1, int64_t>::from_bits(bits);
    }
};


static constexpr size_t ISR_FREQ = 20000;
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

    // static dsp::CommandShaper1 command_shaper_{{
    //     .kp = tau * tau,
    //     .kd = 2 * tau,
    //     .max_spd = 40.0_r,
    //     // .max_acc = 200.0_r,
    //     // .max_acc = 80.0_r,
    //     .max_acc = 100.0_r,
    //     .fs = 1000
    // }};

    static constexpr auto coeffs = typename NonlinearTrackingDifferentor::Config{
        .fs = ISR_FREQ ,
        // .r = 30.5_q24,
        // .h = 2.5_q24
        // .r = 252.5_iq10,
        .r = 152.5_iq10,
        .h = 0.012_iq10
    }.try_to_coeffs().unwrap();
    static NonlinearTrackingDifferentor command_shaper_{
        coeffs
    };

    iq16 u = 0;
    Microseconds elapsed_micros = 0us;


    static constexpr auto track_coeffs = LinearTrackingDifferentiator<iq16, 2>::Config{
        .fs = ISR_FREQ , .r = 40
    }.try_to_coeffs().unwrap();

    [[maybe_unused]] LinearTrackingDifferentiator<iq16, 2> pos_tracker_{
        track_coeffs
    };

    [[maybe_unused]]
    auto command_shaper_poller = [&](){

        const auto u0 = clock::micros();
        command_shaper_.update(u);
        const auto u1 = clock::micros();
        elapsed_micros = u1 - u0;

        // leso.update(command_shaper_.state()[0], u);

    };

    auto & timer = hal::timer1;
    timer.init({
        .count_freq = hal::NearestFreq(ISR_FREQ ),
        .count_mode = hal::TimerCountMode::Up
    }, EN);


    timer.register_nvic<hal::TimerIT::Update>({0,0}, EN);
    timer.enable_interrupt<hal::TimerIT::Update>(EN);
    timer.set_event_handler([&](hal::TimerEvent ev){
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
        u = 13010 + 10 * sign(iq16(sinpu(ctime * 0.5_r)));
        // u = 10 * sign(iq16(sinpu(ctime * 0.5_r)));

        DEBUG_PRINTLN(
            u,
            iq16::from_bits(command_shaper_.state().z1.to_bits() >> 16),
            iq16::from_bits(command_shaper_.state().z2.to_bits() >> 16),
            elapsed_micros.count()
            // leso.get_disturbance()
        );

        clock::delay(2ms);
    }

}