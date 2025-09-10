#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"
#include "core/magic/size_traits.hpp"
#include "core/magic/function_traits.hpp"
#include "core/magic/serialize_traits.hpp"
#include "core/magic/enum_traits.hpp"


#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "hal/bus/uart/uarthw.hpp"

#include "drivers/wireless/Radio/LT8960/LT8960L.hpp"

#include "digipw/SVPWM/svpwm3.hpp"



using namespace ymd;
using namespace ymd::hal;

using namespace ymd::drivers;


#define DBG_UART hal::uart2


static constexpr auto ch = LT8960L::Channel(76);
static constexpr uint32_t master_id = 65536;

// static constexpr size_t TX_FREQ = 500;
static constexpr size_t TX_FREQ = 4000;
static constexpr size_t RX_FREQ = TX_FREQ;


static uint32_t get_id(){
    // const auto id = uint32_t(sys::Chip::getChipId());
    const auto size = sys::chip::get_flash_size();
    return size;
}

static bool has_tx_authority(){
    return get_id() == master_id;
}

static bool has_rx_authority(){
    return get_id() == master_id;
}


class LFSR {
public:
    static constexpr uint32_t DEFAULT_SEED = 0xACE12345;
    constexpr LFSR(uint32_t seed = DEFAULT_SEED) : state(seed) {}

    constexpr uint8_t next() {
        uint8_t bit = (state & 1) ^ ((state >> 1) & 1) ^ ((state >> 2) & 1) ^ ((state >> 3) & 1);
        state = (state >> 1) | (bit << 15);
        return bit;
    }

private:
    uint32_t state;
};

// 扰砝函数
void scramble(std::span<uint8_t> data) {
    LFSR lfsr;
    for (auto& byte : data) {
        for (size_t i = 0; i < 8; ++i) {
            byte ^= uint8_t(lfsr.next() << i);
        }
    }
}

// 坝扰砝函�?
void descramble(std::span<uint8_t> data) {
    LFSR lfsr;
    for (auto& byte : data) {
        for (size_t i = 0; i < 8; ++i) {
            byte ^= uint8_t(lfsr.next() << i);
        }
    }
}

static constexpr uint8_t calc_crc(const std::span<const uint8_t> pbuf){
    uint8_t sum = 0;
    for(size_t i = 0; i < pbuf.size(); i++){
        sum += uint8_t(pbuf[i]);
    }
    return sum;
};


template<typename ... Ts>
static constexpr auto make_payload_from_args(Ts && ... args){
    const auto body = magic::serialize_args_to_bytes(args...);
    const auto crc = calc_crc(std::span(body));

    constexpr size_t size = magic::total_bytes_of_args_v<std::decay_t<Ts> ... > + 1;
    std::array<uint8_t, size> payload;
    std::copy(body.begin(), body.end(), payload.begin());
    payload[body.size()] = uint8_t{crc};

    return payload;
};

template<typename ... Ts>
static constexpr Option<std::tuple<Ts...>> make_tuple_from_payload(std::span<const uint8_t> pbuf){
    auto crc = calc_crc(pbuf.subspan(0, pbuf.size() - 1));
    if (pbuf.back() != uint8_t{crc}){
        return None;
    }

    return Some(magic::make_tuple_from_bytes<std::tuple<Ts...>>(
        pbuf.subspan(0, pbuf.size() - 1)));
}

void lt8960_tb(){

    auto led = hal::PC<13>();
    led.outpp();

    auto scl1_gpio = hal::PB<6>();
    auto sda1_gpio = hal::PB<7>();

    auto scl2_gpio = hal::PA<9>();
    auto sda2_gpio = hal::PA<10>();

    LT8960L tx_ltr{&scl1_gpio, &sda1_gpio};
    LT8960L rx_ltr{&scl2_gpio, &sda2_gpio};
    
    auto common_settings = [](LT8960L & ltr) -> Result<void, LT8960L::Error>{
        static constexpr auto DATA_RATE = LT8960L::DataRate::_62_5K;
        static constexpr auto TOLERANCE = 1;
        static constexpr auto PACKET_TYPE = LT8960L::PacketType::Manchester;
        if(const auto res = ltr.set_rf_channel(ch);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = ltr.enable_use_hw_pkt(EN);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = ltr.set_datarate(DATA_RATE);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = ltr.enable_gain_weaken(EN);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = ltr.set_syncword_tolerance_bits(TOLERANCE);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = ltr.set_pack_type(PACKET_TYPE);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = ltr.set_retrans_time(3);
            res.is_err()) return Err(res.unwrap_err());
        if(const auto res = ltr.enable_autoack(DISEN);
            res.is_err()) return Err(res.unwrap_err());
        return Ok();
    };

    if(has_rx_authority()) {
        rx_ltr.init(LT8960L::Config::from_default()).examine();
        common_settings(rx_ltr).examine();
    }

    if(has_tx_authority()){
        tx_ltr.init(LT8960L::Config::from_default()).examine();

        common_settings(tx_ltr).examine();
    }

    DEBUG_PRINTLN("LT8960L init done");
    
    auto tx_task = [&]{
        // if(!tx_ltr.is_pkt_ready().unwrap()) return;
        // std::array data = {uint8_t(uint8_t(64 + 64 * sin(clock::time() * 20))), uint8_t(0x34), uint8_t(0x56), uint8_t(0x78)};
        const auto t = clock::time();
        const auto [s, c] = sincos(frac(t) * tau);
        // auto [u, v, w] = SVM(s,c);
        // const auto payload = serialize_args_to_bytes(u, v, t);
        // auto copy_arr_to_span[](std::span<uint8_t> dest, const std::array<uint8_t, auto>& src){
        //     std::copy(src.begin(), src.end(), dest.begin());
        // };

        const auto payload = make_payload_from_args(
            // uint8_t(sin(t * 40) * 30 + 50),
            // uint8_t(cos(t * 30) * 30 + 50)
            // uint8_t(0x56),
            // uint8_t(0x78)
            s,c
        );

        tx_ltr.transmit_rf(std::span(payload)).unwrap();
        led.toggle();
    };
    
    // real_t mdur;
    [[maybe_unused]] auto rx_task = [&]{
        static std::array<uint8_t, 16> buf;

        // const real_t mbegin = micros();
        auto len = rx_ltr.receive_rf(buf).unwrap();
        auto data = std::span(buf).subspan(0, len);
        if(len){
            // auto mend = micros();
            // auto [u, v, w] = make_tuple_from_bytes<std::tuple<real_t, real_t, real_t>>(std::span<const uint8_t>(buf));
            // auto [u, v, w] = make_tuple_from_bytes<std::tuple<real_t, real_t, real_t>>(std::span<const uint8_t>(buf));
            // auto [u] = make_tuple_from_bytes<std::tuple<real_t>>(std::span<const uint8_t>(data));
            auto may_res = make_tuple_from_payload<real_t, real_t>(std::span<const uint8_t>(data));
            // DEBUG_PRINTLN(u, v, w, clock::time() - tt);
            // DEBUG_PRINTLN(u, v, clock::time() - w, mend -  mbegin);
            // DEBUG_PRINTLN(std::dec, u, mend -  mbegin, std::hex, std::showbase, data);
            // DEBUG_PRINTLN(std::dec, u, mend -  mbegin, data);
            if(may_res.is_some()) {
                auto [u, v] = may_res.unwrap();
                DEBUG_PRINTLN(std::dec, u, v);
            }
        }
        led.toggle();
    };



    if (has_tx_authority()) {
        hal::timer1.init({.freq = TX_FREQ}, EN);
        hal::timer1.attach(hal::TimerIT::Update, {0,0}, tx_task, EN);
    }

    clock::delay(5ms);

    if (has_rx_authority()) {
        hal::timer2.init({RX_FREQ}, EN);
        hal::timer2.attach(hal::TimerIT::Update, {0,1}, rx_task, EN);
    }

    while(true){
        __WFI();
        // DEBUG_PRINT(".");
        // for(auto i : std::views::iota(0, 2)){
        //     DEBUG_PRINTLN(i);
        // }
            // DEBUG_PRINTLN(micros());
    }
}

void lt8960_main(){
    // DBG_UART.init({576_KHz});
    // DBG_UART.init(1152_KHz);
    DBG_UART.init({6_MHz});
    DEBUGGER.retarget(&DBG_UART);
    DEBUGGER.no_brackets(EN);

    lt8960_tb();
}