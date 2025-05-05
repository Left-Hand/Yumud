#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/clock/time.hpp"


#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/wireless/Radio/LT8960/LT8960L.hpp"
#include "src/testbench/algo/utils.hpp"

#include "hal/bus/uart/uarthw.hpp"

#include "digipw/SVPWM/svpwm3.hpp"
#include "hal/gpio/gpio_port.hpp"


using namespace ymd::drivers;


#define UART hal::uart2

#define MAG_ACTIVATED

#define let const auto



scexpr auto ch = LT8960L::Channel(76);
static constexpr uint32_t master_id = 65536;

// static constexpr size_t TX_FREQ = 500;
static constexpr size_t TX_FREQ = 4000;
static constexpr size_t RX_FREQ = TX_FREQ;


uint32_t get_id(){
    // const auto id = uint32_t(sys::Chip::getChipId());
    const auto size = sys::chip::get_flash_size();
    return size;
}

bool has_tx_authority(){
    return get_id() == master_id;
}

bool has_rx_authority(){
    return get_id() == master_id;
}


// 伪隝机庝列生戝器（简坕线性坝馈移佝寄存器�?
class LFSR {
public:
    LFSR(uint32_t seed = 0xACE12345) : state(seed) {}

    uint8_t next() {
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

static constexpr uint8_t calc_crc(const std::span<const uint8_t> pdata){
    uint8_t sum = 0;
    for(size_t i = 0; i < pdata.size(); i++){
        sum += uint8_t(pdata[i]);
    }
    return sum;
};


template<typename ... Ts>
auto make_payload_from_args(Ts && ... args){
    const auto body = make_bytes_from_args(args...);
    const auto crc = calc_crc(std::span(body));

    constexpr size_t size = total_bytes_v<std::decay_t<Ts> ... > + 1;
    std::array<uint8_t, size> payload;
    std::copy(body.begin(), body.end(), payload.begin());
    payload[body.size()] = uint8_t{crc};

    return payload;
};

template<typename ... Ts>
Option<std::tuple<Ts...>> make_tuple_from_payload(std::span<const uint8_t> pdata){
    auto crc = calc_crc(pdata.subspan(0, pdata.size() - 1));
    if (pdata.back() != uint8_t{crc}){
        return None;
    }

    return Some(make_tuple_from_bytes<std::tuple<Ts...>>(pdata.subspan(0, pdata.size() - 1)));
}

void lt8960_tb(){

    auto & led = portC[13];
    led.outpp();


    LT8960L tx_ltr{portB[6], portB[7]};
    LT8960L rx_ltr{portA[9], portA[10]};
    
    auto common_settings = [](LT8960L & ltr){
        (ltr.set_rf_channel(ch)
        | ltr.enable_use_hw_pkt(true)
        | ltr.set_datarate(LT8960L::DataRate::_250K)
        // | ltr.set_datarate(LT8960L::DataRate::_62_5K)
        // | ltr.set_datarate(LT8960L::DataRate::_1M)
        | ltr.enable_gain_weaken(true)
        // | ltr.set_syncword_tolerance_bits(1)
        | ltr.set_syncword_tolerance_bits(0)
        | ltr.set_pack_type(LT8960L::PacketType::Manchester)
        // | ltr.set_retrans_time(3)
        // | ltr.enable_autoack(false)

        ).unwrap();
    };

    if(has_rx_authority()) {
        rx_ltr.init(LT8960L::Power::_8_Db, 0x12345678).loc().expect("RX init failed!");
        common_settings(rx_ltr);
    }
    if(has_tx_authority()){
        tx_ltr.init(LT8960L::Power::_n13_Db, 0x12345678).loc().expect("TX init failed!");

        common_settings(tx_ltr);
    }

    DEBUG_PRINTLN("LT8960L init done");
    
    auto tx_task = [&]{
        // if(!tx_ltr.is_pkt_ready().unwrap()) return;
        // std::array data = {uint8_t(uint8_t(64 + 64 * sin(time() * 20))), uint8_t(0x34), uint8_t(0x56), uint8_t(0x78)};
        const auto t = time();
        const auto [s, c] = sincos(frac(t) * tau);
        // auto [u, v, w] = SVM(s,c);
        // const auto payload = make_bytes_from_args(u, v, t);
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
            // DEBUG_PRINTLN(u, v, w, time() - tt);
            // DEBUG_PRINTLN(u, v, time() - w, mend -  mbegin);
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
        hal::timer1.init(TX_FREQ);
        hal::timer1.attach(hal::TimerIT::Update, {0,0}, tx_task);
    }

    delay(5);

    if (has_rx_authority()) {
        hal::timer2.init(RX_FREQ);
        hal::timer2.attach(hal::TimerIT::Update, {0,1}, rx_task);
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
    // UART.init(576_KHz);
    // UART.init(1152_KHz);
    UART.init(6_MHz);
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets();

    lt8960_tb();
}