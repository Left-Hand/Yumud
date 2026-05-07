#pragma once

#include "core/math/real.hpp"
#include <span>

namespace ymd::hal{
    struct TimerOC;
    struct DmaChannel;
}

namespace ymd::drivers::dshot{

static constexpr size_t DSHOT_LEN = 40;



enum struct [[nodiscard]] Command : uint8_t {
    MOTOR_STOP = 0, // not currently implemented
    BEEP1 = 1, // Wait at least the length of the beep before the next command (260 milliseconds)
    BEEP2 = 2, // Wait at least the length of the beep before the next command (260 milliseconds)
    BEEP3 = 3, // Wait at least the length of the beep before the next command (260 milliseconds)
    BEEP4 = 4, // Wait at least the length of the beep before the next command (260 milliseconds)
    BEEP5 = 5, // Wait at least the length of the beep before the next command (260 milliseconds)
    ESC_INFO = 6, // Wait at least 12 milliseconds before the next command.
    SPIN_DIRECTION_1 = 7, // 6 times required
    SPIN_DIRECTION_2 = 8, // 6x required
    _3D_MODE_OFF = 9, // 6 times required
    _3D_MODE_ON = 10, // 6 times required
    SETTINGS_REQUEST = 11, // Not implemented at this time.
    SAVE_SETTINGS = 12, // 6 times required, wait at least 35 milliseconds before next command
    DSHOT_EXTENDED_TELEMETRY_ENABLE = 13, // 6x required (only on EDT-enabled firmware)
    DSHOT_EXTENDED_TELEMETRY_DISABLE = 14, // 6x required (only on EDT-enabled firmware)
    
    SPIN_DIRECTION_NORMAL = 20, // 6 times required
    SPIN_DIRECTION_REVERSED = 21, // 6x required
    LED0_ON = 22, // -
    LED1_ON = 23, // -
    LED2_ON = 24, // -
    LED3_ON = 25, // -
    LED0_off = 26, // -
    LED1_OFF = 27, // - LED2_OFF = 28, // -
    LED2_off = 28, // - led3_off = 29, // -
    LED3_OFF = 29, // - LED1_OFF = 27, // - LED2_OFF = 28, // -
    AUDIO_STREAM_MODE = 30, // Audio_Stream mode on/off not currently implemented
    MUTE_MODE = 31, // Mute mode on/off not currently implemented
    SIGNAL_LINE_TELEMETRY_DISABLE = 32, // Required 6 times. Disable commands 42 through 47
    SIGNAL_LINE_TELEMETRY_ENABLE = 33, // Required 6 times. Enable commands 42 through
};

[[nodiscard]] static constexpr uint16_t calc_crc(uint16_t bits_in){
    const uint16_t speed_bits = bits_in << 5;
    const uint16_t bits = bits_in << 1;
    return speed_bits | static_cast<uint16_t>((bits ^ (bits >> 4) ^ (bits >> 8)) & 0x0f);
}

struct [[nodiscard]] DshotContentBuilder final{
    uint16_t low_duty;
    uint16_t high_duty;

    static constexpr DshotContentBuilder from_default(){
        constexpr uint16_t HIGH_CVR = (234 * 2 / 3);
        constexpr uint16_t LOW_CVR = (234 * 1 / 3);

        return {
            .low_duty = LOW_CVR,
            .high_duty = HIGH_CVR
        };
    }



    void exact_to(std::span<uint16_t, DSHOT_LEN> buf, const uint16_t duty) const {
        if(duty > 2048) __builtin_trap();


        if(duty){
            auto bits_with_crc = calc_crc(duty);
            exact_to_unchecked(std::span(buf), bits_with_crc);
        }else{
            std::fill(buf.begin(), buf.end(), uint16_t(0));
        }
    }

    void exact_to_unchecked(std::span<uint16_t, DSHOT_LEN> buf, const uint16_t duty) const {
        uint16_t mut_bits = duty;
        for(size_t i = 0; i < 16; i++){
            buf[i] = (mut_bits & 0x8000) ? high_duty : low_duty;
            mut_bits = mut_bits << 1;
        }
    }
};


struct BurstDmaPwm final{
public:
    BurstDmaPwm(hal::TimerOC & timer_oc);

    void set_buf(std::span<const uint16_t> pbuf);
    void invoke();
    void install();
    bool is_done();
    // uint32_t calc_cvr_from_duty(const uq32 duty) const;
    // uq8 get_period_us() const;
private:
    hal::TimerOC & timer_oc_;
    hal::DmaChannel & dma_channel_;
    std::span<const uint16_t> pbuf_;
};


struct DShotChannel{
public:
    explicit DShotChannel(hal::TimerOC & oc);
    explicit DShotChannel(const DShotChannel & other) = delete;
    explicit DShotChannel(DShotChannel && other) = delete;

    void init();

    void set_content(const uint16_t content_bits);

// private:
public:



    std::array<uint16_t, DSHOT_LEN> buf_ = {0};
    BurstDmaPwm burst_dma_pwm_;

    // static uint16_t calculate_crc(uint16_t data_in);

    // static void update(std::span<uint16_t, DSHOT_LEN> buf, uint16_t data);

    // static void clear(std::span<uint16_t, DSHOT_LEN> buf);

    void invoke();
};

}