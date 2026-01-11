#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/clock.hpp"
#include "core/clock/time.hpp"
#include "core/utils/zero.hpp"

#include "hal/bus/uart/uarthw.hpp"
#include "hal/timer/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "primitive/arithmetic/angular.hpp"
#include "core/string/string_view.hpp"
#include "dsp_lpf.hpp"
using namespace ymd;

static constexpr size_t FOC_FREQ = 16000;
static constexpr auto DT = uq32::from_rcp(FOC_FREQ);

static constexpr size_t PLL_PI_FC = 200;

[[maybe_unused]] static uint32_t generate_noise(){
    static uint32_t seed = 0;
    seed = seed * 214013 + 2531011;
    return seed;
}





#ifndef M_PI
#define M_PI (3.1415926536)
#endif

#define TYPE_PU         (0)
#define TYPE_RAD        (1)


namespace ymd::myiqmath::details{

struct [[nodiscard]] Atan2Flag{
    /*
    * Extract the sign from the inputs and set the following flag bits:
    *
    *      flag = xxxxxTQS
    *      x = unused
    *      T = transform was inversed
    *      Q = 2nd or 3rd quadrant (-x)
    *      S = sign bit needs to be set (-y)
    */

    uint8_t y_is_neg:1;
    uint8_t x_is_neg:1;
    uint8_t inversed:1;

    static constexpr Atan2Flag zero(){
        return Atan2Flag{0, 0, 0};
    }


    template<size_t Q, int type>
    [[nodiscard]] constexpr int32_t apply_to_uiq32pu(uint32_t uiq32ResultPU) const {
        auto & self = *this;
        int32_t iqNResult;
        int32_t iq29Result;

        /* Check if we inversed the transformation. */
        if (self.inversed) {
            /* atan(y/x) = pi/2 - uiq32ResultPU */
            uiq32ResultPU = (uint32_t)(0x40000000 - uiq32ResultPU);
        }

        /* Check if the result needs to be mirrored to the 2nd/3rd quadrants. */
        if (self.x_is_neg) {
            /* atan(y/x) = pi - uiq32ResultPU */
            uiq32ResultPU = (uint32_t)(0x80000000 - uiq32ResultPU);
        }

        /* Round and convert result to correct format (radians/PU and iqN type). */
        if constexpr(type ==  TYPE_PU) {
            uiq32ResultPU += (uint32_t)1 << (31 - Q);
            iqNResult = uiq32ResultPU >> (32 - Q);
        }

        /* Set the sign bit and result to correct quadrant. */
        if (self.y_is_neg) {
            return -iqNResult;
        } else {
            return iqNResult;
        }
    };

    [[nodiscard]] constexpr uint8_t to_u8() const {
        return std::bit_cast<uint8_t>(*this);
    }
};

struct [[nodiscard]] Atan2Intermediate{
    using Self = Atan2Intermediate;


    // * Calculate atan2 using a 3rd order Taylor series. The coefficients are stored
    // * in a lookup table with 17 ranges to give an accuracy of XX bits.
    // *
    // * The input to the Taylor series is the ratio of the two inputs and must be
    // * in the range of 0 <= input <= 1. If the y argument is larger than the x
    // * argument we must apply the following transformation:
    // *
    // *     atan(y/x) = pi/2 - atan(x/y)
    // */
    static constexpr uint32_t transfrom_uq31_x_to_uq32_result(uint32_t uiq31Input) {
        const auto * piq32Coeffs = &iqmath::details::_IQ32atan_coeffs[(uiq31Input >> 24) & 0x00fc];
        /*
        * Calculate atan(x) using the following Taylor series:
        *
        *     atan(x) = ((c3*x + c2)*x + c1)*x + c0
        */

        /* c3*x */
        uint32_t uiq32ResultPU = iqmath::details::__mpyf_l(uiq31Input, *piq32Coeffs++);

        /* c3*x + c2 */
        uiq32ResultPU = uiq32ResultPU + *piq32Coeffs++;

        /* (c3*x + c2)*x */
        uiq32ResultPU = iqmath::details::__mpyf_l(uiq31Input, uiq32ResultPU);

        /* (c3*x + c2)*x + c1 */
        uiq32ResultPU = uiq32ResultPU + *piq32Coeffs++;

        /* ((c3*x + c2)*x + c1)*x */
        uiq32ResultPU = iqmath::details::__mpyf_l(uiq31Input, uiq32ResultPU);

        /* ((c3*x + c2)*x + c1)*x + c0 */
        uiq32ResultPU = uiq32ResultPU + *piq32Coeffs++;
        return uiq32ResultPU;
    }
};

std::tuple<Atan2Flag, uint32_t> convert_to_flag(uint32_t uiqNInputX, uint32_t uiqNInputY){
    Atan2Flag flag = Atan2Flag::zero();
    uint32_t uiq31Input;

    if (uiqNInputY & (1U << 31)) {
        flag.y_is_neg = 1;
        uiqNInputY = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(uiqNInputY));
    }

    if (uiqNInputX & (1U << 31)) {
        flag.x_is_neg = 1;
        uiqNInputX = std::bit_cast<uint32_t>(-std::bit_cast<int32_t>(uiqNInputX));
    }

    /*
    * Calcualte the ratio of the inputs in iq31. When using the iq31 div
    * fucntions with inputs of matching type the result will be iq31:
    *
    *     iq31 = _IQ31div(iqN, iqN);
    */
    if (uiqNInputX < uiqNInputY) {
        flag.inversed = 1;
        uiq31Input = std::bit_cast<uint32_t>(iqmath::details::__IQNdiv_impl<31, true>(
            uiqNInputX, uiqNInputY));
    } else if((uiqNInputX > uiqNInputY)) {
        uiq31Input = std::bit_cast<uint32_t>(iqmath::details::__IQNdiv_impl<31, true>(
            uiqNInputY, uiqNInputX));
    } else{
        // 1/8 lap
        // 1/8 * 2^32
        // return flag.template apply_to_uiq32pu<Q, type>(((1u << (32 - 3))));
        uiq31Input = (1u << (32 - 3));
    }
    return std::make_tuple(flag, uiq31Input);
}

template<const size_t Q, const uint8_t type>
constexpr int32_t __IQNatan2_impl(uint32_t uiqNInputY, uint32_t uiqNInputX){
    const auto [flag, uiq31Input] = convert_to_flag(uiqNInputX, uiqNInputY);
    
    const uint32_t uiq32ResultPU = Atan2Intermediate::transfrom_uq31_x_to_uq32_result(uiq31Input);
    return flag.template apply_to_uiq32pu<Q, type>(uiq32ResultPU);
}

template<const size_t Q>
constexpr math::fixed_t<Q, int32_t> test_atan(
    math::fixed_t<Q, int32_t> iqNInputY, 
    math::fixed_t<Q, int32_t> iqNInputX)
{
    return math::fixed_t<Q, int32_t>::from_bits(__IQNatan2_impl<Q, TYPE_RAD>(
        std::bit_cast<uint32_t>(iqNInputY.to_bits()), 
        std::bit_cast<uint32_t>(iqNInputX.to_bits()))
    );

    // Atan2Intermediate::transfrom_uq31_x_to_uq32_result
}
template<const size_t Q, const uint8_t type>
constexpr int32_t __IQNatan_impl(uint32_t uiqNInputX){
    return __IQNatan2_impl<Q, type>(uiqNInputX, (1u << Q));
}


template<const size_t Q>
constexpr math::fixed_t<Q, int32_t> _IQNatan2(
    math::fixed_t<Q, int32_t> iqNInputY, 
    math::fixed_t<Q, int32_t> iqNInputX
){
    return math::fixed_t<Q, int32_t>::from_bits(__IQNatan2_impl<Q, TYPE_RAD>(
        std::bit_cast<uint32_t>(iqNInputY.to_bits()), 
        std::bit_cast<uint32_t>(iqNInputX.to_bits()))
    );
}

template<const size_t Q>
constexpr math::fixed_t<Q, int32_t> _IQNatan2PU(
    math::fixed_t<Q, int32_t> iqNInputY, 
    math::fixed_t<Q, int32_t> iqNInputX
){
    return math::fixed_t<Q, int32_t>::from_bits(__IQNatan2_impl<Q, TYPE_PU>(
        std::bit_cast<uint32_t>(iqNInputY.to_bits()), 
        std::bit_cast<uint32_t>(iqNInputX.to_bits()))
    );
}

template<const size_t Q>
constexpr math::fixed_t<Q, int32_t> _IQNatan2(math::fixed_t<Q, int32_t> iqNInputX){
    return math::fixed_t<Q, int32_t>::from_bits(__IQNatan_impl<Q, TYPE_RAD>(
        std::bit_cast<uint32_t>(iqNInputX.to_bits()))
    );
}

template<const size_t Q>
constexpr math::fixed_t<Q, int32_t> _IQNatan2PU(math::fixed_t<Q, int32_t> iqNInputX){
    return math::fixed_t<Q, int32_t>::from_bits(__IQNatanPU_impl<Q, TYPE_PU>(
        std::bit_cast<uint32_t>(iqNInputX.to_bits()))
    );
}

}

void sincospll_main(){
    // DEBUGGER_INST.init(DEBUG_UART_BAUD, CommStrategy::Blocking);
    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576_KHz),
        .tx_strategy = CommStrategy::Blocking,
    });
    DEBUGGER.retarget(&DEBUGGER_INST);
    DEBUGGER.no_brackets(EN);
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    hal::timer2.init({
        .remap = hal::TIM2_REMAP_A0_A1_A2_A3,
        .count_freq =  hal::NearestFreq(FOC_FREQ),
        .count_mode = hal::TimerCountMode::Up
    }).unwrap().dont_alter_to_pins();
    hal::timer2.register_nvic<hal::TimerIT::Update>({0,0}, EN);
    hal::timer2.enable_interrupt<hal::TimerIT::Update>(EN);

    Angular<uq32> simulated_angle_ = Zero;

    iq16 normalized_sine_ = Zero;
    iq16 normalized_cosine_ = Zero;
    
    Angular<iq16> computed_angluar_speed_ = Zero;
    Angular<uq32> computed_angle_ = Zero;

    iq16 err_filtered_ = Zero;
    auto isr_fn = [&]{
        {//simulate input
            [[maybe_unused]] const iq16 now_secs = clock::time();
            const iq16 speed = 50 * iq16(math::sin(now_secs)) + 2 * iq16(math::sin(16 * now_secs));
            // const iq16 speed = 4;
            // const iq16 speed = 2;
            simulated_angle_ = simulated_angle_.from_turns(uq32::from_bits(static_cast<uint32_t>(
                static_cast<int64_t>(simulated_angle_.to_turns().to_bits()) + (
                (static_cast<int64_t>(DT.to_bits()) * speed.to_bits()) >> 16)
            )));
            const auto [sine_, cosine_] = simulated_angle_.sincos();
            const auto [noise_sine_, noise_cosine_] = [] -> std::tuple<iq16, iq16>{
                const uint32_t noise = generate_noise();
                const int32_t i1 = std::bit_cast<int16_t>(static_cast<uint16_t>(noise & 0x07fF));
                const int32_t i2 = std::bit_cast<int16_t>(static_cast<uint16_t>((noise >> 16) & 0x07fF));
                return std::make_tuple(
                    iq16::from_bits(i1),
                    iq16::from_bits(i2)
                );
                // return std::make_tuple(
                //     iq16::from_bits(0),
                //     iq16::from_bits(0)
                // );
            }();

            const auto measured_sine = iq16(sine_) + noise_sine_;
            const auto measured_cosine = iq16(cosine_) + noise_cosine_;

            normalized_sine_ = dsp::lpf_exprimetal(normalized_sine_, measured_sine);
            normalized_cosine_ = dsp::lpf_exprimetal(normalized_cosine_, measured_cosine);
            // normalized_cosine_ = normalized_cosine_ * 0.9_iq16;
            // normalized_cosine_ = normalized_cosine_ + 0.1_iq16;
        }
        
        // static constexpr size_t PLL_PI_FC = 1400;

        // static constexpr size_t PLL_PI_FC = 4;
        static constexpr size_t KP = 2 * PLL_PI_FC;
        static constexpr size_t KI = PLL_PI_FC * PLL_PI_FC;
        static constexpr uq16 KI_BY_FS = uq16::from_bits((KI * (1u << 16)) / FOC_FREQ);

        const auto [sine_, cosine_] = computed_angle_.sincos();
        const iq16 e = ((cosine_) * normalized_sine_- (sine_) * normalized_cosine_);
        err_filtered_ = dsp::lpf_exprimetal(err_filtered_, e);
        // const iq16 e = (simulated_angle_.to_turns() - computed_angle_.to_turns());
        // computed_angluar_speed_ = Angular<iq16>::from_turns(1);
        computed_angluar_speed_ = computed_angluar_speed_.from_turns(
            computed_angluar_speed_.to_turns() + e * KI_BY_FS);

        uint32_t bits = computed_angle_.to_turns().to_bits();
        bits += static_cast<uint32_t>((
            static_cast<uint64_t>(DT.to_bits()) * ((KP * e + computed_angluar_speed_.to_turns()).to_bits())
        ) >> 16);
        computed_angle_ = computed_angle_.from_turns(
            uq32::from_bits(bits)
        );
    };

    hal::timer2.set_event_handler([&](const hal::TimerEvent & event){
        switch(event){
            case hal::TimerEvent::Update:
                isr_fn();
                break;
            default:
                break;
        }
    });

    hal::timer2.start();

    while(true){
        DEBUG_PRINTLN(
            simulated_angle_.to_turns(),
            normalized_sine_, 
            normalized_cosine_,

            computed_angluar_speed_.to_turns(),
            computed_angle_.to_turns(),
            err_filtered_,
            Angular<iq16>::from_turns(iq16(simulated_angle_.to_turns()) - iq16(computed_angle_.to_turns())).signed_normalized().to_turns(),
            // (computed_angle_ + Angular<uq32>::from_turns(0.125_uq32)).unsigned_normalized().to_turns()
            // myiqmath::details::test(normalized_sine_, normalized_cosine_)
            // iq31::from_bits(myiqmath::details::Atan2Intermediate::transfrom_uq31_x_to_uq32_result(simulated_angle_.to_turns().to_bits() >> 1))

            // math::atan2pu(normalized_sine_, normalized_cosine_)
            (computed_angle_ + dsp::calc_lpf_phaseshift_uq32(800, computed_angluar_speed_.to_turns())).unsigned_normalized().to_turns()
            // (computed_angle_ + Angular<uq32>::from_turns(uq32::from_bits(static_cast<int32_t>(static_cast<int64_t>(computed_angluar_speed_.to_turns().to_bits() << 16) / 800)))).unsigned_normalized().to_turns()
            
        );
    }
}