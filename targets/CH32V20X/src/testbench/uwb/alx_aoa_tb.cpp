#include "src/testbench/tb.h"

#include "hal/conn/uart/hw_singleton.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/dma/dma.hpp"

#include "core/clock/clock.hpp"
#include "core/clock/time.hpp"
#include "core/debug/debug.hpp"
#include "core/async/timer.hpp"
#include "core/utils/zero.hpp"
#include "core/string/view/string_view.hpp"

#include "core/utils/result.hpp"
#include "core/container/atomic_bitset.hpp"
#include "geometry.hpp"
#include "drivers/Proximeter/MK8000TR/mk8000tr_stream.hpp"
#include "drivers/Proximeter/ALX_AOA/alx_aoa.hpp"

#include "middlewares/algebra/regions/ray2.hpp"
#include <random>

using namespace ymd;
using namespace ymd::drivers;

// #define DEBUGGER_INST hal::usart2

using AlxEvent = drivers::alx_aoa::Event;
using AlxError = drivers::alx_aoa::Error;

using AlxLocation = drivers::alx_aoa::Location;
using AlxHeartBeat = drivers::alx_aoa::HeartBeat;
using drivers::mk8000tr::MK8000TR_ParseReceiver;
using drivers::alx_aoa::AlxAoa_ParseReceiver;
using Mk8Event = drivers::mk8000tr::Event;

using AlxMeasurement = math::SphericalCoordinates<float>;



namespace alx_aoa_tb{
struct [[nodiscard]] Mk8Measurement{
    using Self = Mk8Measurement;

    float distance;
    float strength;

    static constexpr Self zero(){
        return Self{
            .distance = 0,
            .strength = 0
        };
    }

    friend OutputStream & operator << (OutputStream & os, const Self & self) noexcept {
        return os
            << os.field("distance")(self.distance) << os.splitter()
            << os.field("strength")(self.strength)
        ;
    }
};


struct BlinkActivity{
    hal::Gpio & blue_led_pin_;
    void resume(){
        blue_led_pin_ = BoolLevel::from((
            uint32_t(clock::millis().count()) % 400) > 200);
    }
};

struct AlxActivity{
    hal::Uart & uart_;
    drivers::alx_aoa::AlxAoa_ParseReceiver & parser_;
    uint32_t received_bytes_cnt_ = 0;

    void resume(){
        if(uart_.available() == 0) return;
        while(uart_.available()){
            uint8_t byte;
            const auto len = uart_.try_read_byte(byte);
            if(len == 0) break;
            // _bytes.push_back(uint8_t(byte));
            parser_.push_byte(static_cast<uint8_t>(byte));
            received_bytes_cnt_++;
        }
    }
};
}

using namespace alx_aoa_tb;


class MarkovNoiseGen {
public:
    struct Config {
        float a;     // 转移系数
        float sigma; // 高斯噪声标准差
    };

    explicit MarkovNoiseGen(const Config& cfg, uint32_t seed = 123456789u)
        : a_(cfg.a)
        , sigma_(cfg.sigma)
        , last_(0.0f)
        , seed_(seed)
        , spare_has_(false)
        , spare_(0.0f)
    {}

    // 生成下一个马尔可夫噪声值
    float get_next() {
        float w = gaussian() * sigma_;   // N(0, sigma^2)
        last_ = a_ * last_ + w;
        return last_;
    }

private:
    // ------------------- 均匀随机数生成器 (LCG) -------------------
    uint32_t lcg() {
        // 参数采用 glibc 使用的 LCG:  a=1103515245, c=12345, m=2^31
        seed_ = seed_ * 1103515245u + 12345u;
        return seed_ & 0x7fffffff;       // 返回 [0, 2^31-1]
    }

    // 生成 [0, 1) 范围内的均匀浮点数
    float uniform() {
        return static_cast<float>(lcg()) / 2147483648.0f; // 2^31
    }

    // ------------------- Box-Muller 生成标准正态分布 -------------------
    float gaussian() {
        // 使用缓存机制，一次生成两个独立样本，存储一个供下次使用
        if (spare_has_) {
            spare_has_ = false;
            return spare_;
        } else {
            float u1, u2, r, theta;
            do {
                u1 = uniform(); // (0,1)
                u2 = uniform(); // (0,1)
            } while (u1 == 0.0f); // 避免 log(0)

            r = std::sqrt(-2.0f * std::log(u1));
            theta = 2.0f * 3.14159265358979323846f * u2;
            float z1 = r * std::cos(theta);
            float z2 = r * std::sin(theta);
            spare_ = z2;
            spare_has_ = true;
            return z1;
        }
    }

private:
    float a_;
    float sigma_;
    float last_;
    uint32_t seed_;
    bool spare_has_;
    float spare_;
};
// struct MarkovNoiseGen {
//     struct Config {
//         float a;
//         float sigma;
//     };

//     explicit MarkovNoiseGen(const Config& cfg)
//         : a_(cfg.a)
//         , sigma_(cfg.sigma)
//         , rd_()
//         , gen_(rd_())
//         , norm_(0.0f, sigma_)
//         , last_(0.0f)   // 初始噪声值，可根据需要调整
//     {}

//     [[nodiscard]] float get_next() {
//         float w = norm_(gen_);
//         last_ = a_ * last_ + w;
//         return last_;
//     }

// private:
//     float a_;
//     float sigma_;
//     std::random_device rd_;
//     std::mt19937 gen_;
//     std::normal_distribution<float> norm_;
//     float last_;
// };


static constexpr Result<math::fixed<32, uint32_t>, StringView> calc_lpf_alpha_uq32(uint32_t fs, uint32_t fc){
    constexpr size_t SHIFT_BITS = 9;
    constexpr size_t MAX_FREQ = (1u << (32u - SHIFT_BITS)) / 8;  // div 8 for margin
    
    // 参数检查
    if(fs == 0) return Err(StringView("fs cannot be zero"));
    if(fs >= MAX_FREQ) return Err(StringView("fs overflow")); 
    if(fc >= MAX_FREQ) return Err(StringView("fc overflow"));
    if(fc * 2 >= fs) return Err(StringView("nyquist failed"));

    // 使用安全的乘法，防止中间溢出
    const uint64_t pow2_32_SHIFT = static_cast<uint64_t>(1) << (32 + SHIFT_BITS);
    const uint64_t pow2_SHIFT = static_cast<uint64_t>(1) << SHIFT_BITS;
    
    // 计算分子：fs * 2^(32+SHIFT_BITS)
    const uint64_t num = static_cast<uint64_t>(fs) * pow2_32_SHIFT;
    
    // 计算分母：fs * 2^SHIFT_BITS + fc * TAU * 2^SHIFT_BITS
    const uint64_t fs_term = static_cast<uint64_t>(fs) * pow2_SHIFT;
    
    // 确保 TAU 有足够的精度和适当的缩放
    constexpr uint64_t TAU_SCALED = static_cast<uint64_t>(TAU * (1ull << SHIFT_BITS) + 0.5);
    const uint64_t fc_term = static_cast<uint64_t>(fc) * TAU_SCALED;
    
    const uint64_t den = fs_term + fc_term;
    
    // 防止除零
    if(den == 0) return Err(StringView("denominator is zero"));
    
    return Ok(math::fixed<32, uint32_t>::from_bits(~static_cast<uint32_t>(num / den)));
}

static constexpr size_t FOC_FREQ = 50;


//y[n] = alpha * x[n] + beta * y[n-1]
template<size_t Q>
static constexpr math::fixed<Q, int32_t> lpf_1o(
    const math::fixed<Q, int32_t> x_state, 
    const math::fixed<Q, int32_t> x_new, 
    const uq32 alpha
){
    const uq32 beta = uq32::from_bits(~alpha.to_bits());
    using acc_t = std::conditional_t<std::is_signed_v<int32_t>, int64_t, uint64_t>;
    return math::fixed<Q, int32_t>::from_bits(
        int32_t((static_cast<acc_t>(x_new.to_bits()) * alpha.to_bits()) >> 32)
        + int32_t((static_cast<acc_t>(x_state.to_bits()) * beta.to_bits()) >> 32)
    );
}



template<size_t FC, size_t Q>
static constexpr math::fixed<Q, int32_t> lpf_specified_fc(
    const math::fixed<Q, int32_t> x_state,
    const math::fixed<Q, int32_t> x_new
){
    constexpr auto ALPHA = calc_lpf_alpha_uq32(FOC_FREQ, FC).unwrap();
    return lpf_1o(x_state, x_new, ALPHA);
}


template<size_t Q>
static constexpr math::fixed<Q, int32_t> lpf_10hz(
    math::fixed<Q, int32_t> x_state,
    const math::fixed<Q, int32_t> x_new
){
    return lpf_specified_fc<1>(x_state, x_new);
}


void alx_aoa_main(){


    auto & DBG_UART = hal::usart2;

    hal::usart2.init({
        .remap = hal::USART2_REMAP_PA2_PA3,
        .baudrate = hal::NearestFreq(576000),
        .tx_strategy = CommStrategy::Blocking
    });

    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.build_config()
        .set_eps(5)
        .set_splitter(",")
        .no_brackets(EN) 
        .no_fieldname(EN)
        .force_sync(EN)
        .finalize();

    auto blue_led_pin_ = hal::PC<13>();
    blue_led_pin_.outpp();

    hal::usart1.init({
        .remap = hal::USART1_REMAP_PA9_PA10,
        .baudrate = hal::NearestFreq(alx_aoa::DEFAULT_UART_BAUD),
    });

    [[maybe_unused]] auto & uwb_uart_ = hal::usart1;
    
    iq16 azimuth_radians = 0;
    iq16 azimuth_radians_polluted = 0;
    iq16 distance_meters = 0;
    iq16 distance_meters_polluted = 0;
    int times = 0;


    MarkovNoiseGen azi_noise_gen({
        0.97f,
        0.2f
    });

    MarkovNoiseGen dis_noise_gen({
        0.97f,
        0.2f
    }, 8);

    //处理uwb模块事件


    auto uwb_ev_handler = [&](const Result<AlxEvent, AlxError> & res){ 
        if(res.is_err()){
            [[maybe_unused]] const auto err = res.unwrap_err();
            (void)err;
            return;
        }

        const auto now_millis = clock::millis();
        const auto ev = res.examine();



        if(ev.is<AlxLocation>()){
            //处理定位事件
            const AlxLocation loc = ev.unwrap_as<AlxLocation>();
            // const auto addict = iq16::from(azi_noise_gen.get_next());
            azimuth_radians = lpf_10hz(azimuth_radians, loc.azimuth_code.to_angle<iq16>().to_radians());
            azimuth_radians_polluted = azimuth_radians + iq16::from(0.02f * azi_noise_gen.get_next());
            distance_meters = lpf_10hz(distance_meters, loc.distance_code.to_meters<iq16>());
            distance_meters_polluted = distance_meters + iq16::from(0.02f * dis_noise_gen.get_next());

        }else if(ev.is<AlxHeartBeat>()){
            //处理心跳事件
        }

    };

    auto uwb_parse_receiver_ = AlxAoa_ParseReceiver(
        [&](const Result<AlxEvent, AlxError> & res){
            uwb_ev_handler(res);
        }
    );

    uwb_uart_.set_event_callback([&](const hal::UartEvent & ev){
        auto poll_parser = [&](){
            times ++;
            const auto quantity = uwb_uart_.rx_queue()
                .consume_each([&](const uint8_t byte){
                    uwb_parse_receiver_.push_byte(static_cast<uint8_t>(byte));
                });

            (void)quantity;
        };
        switch(ev.kind()){
            case hal::UartEvent::RxIdle:
                poll_parser();
                uwb_parse_receiver_.reset();
                break;
            case hal::UartEvent::RxBulk:
                poll_parser();
                break;
            default: 
                break;
        }
    });


    auto poll_led = [&]{
        const uint32_t now_millis = static_cast<uint32_t>(clock::millis().count());
        blue_led_pin_.write(BoolLevel::from(now_millis % 200 > 100));
    };


    while(true){

        // blink_activity_.resume();
        poll_led();
        // times++;
        // const auto now_secs = clock::seconds();

        // const auto sine = (iq16)math::sin(now_secs);
        // const auto cosine = (iq16)math::cos(now_secs);
        // const auto ab = digipw::AlphaBetaCoord<iq16>(sine, cosine) * 0.5_iq16;

        // const iq16 polluted = azimuth_radians 
        //     + 
        //     ;
        DEBUG_PRINTLN(
            // digipw::SVM(ab),
            // times,
            azimuth_radians,
            // alx_aoa::TargetAngleCode::from_radians(azimuth_radians_polluted).to_angle<iq16>().to_radians(),
            azimuth_radians_polluted,
            distance_meters,
            distance_meters_polluted,
            // azimuth_radians
            // int(iq10(azimuth_radians) * 1000),
            // int(iq10(distance_meters) * 1000)
            // azimuth_radians.to_bits()
            0
        );
    }

    // #else

    // PANIC{"not supported"};

    // #endif
}
