#pragma once

#include "core/math/real.hpp"
#include <span>

namespace ymd::hal{
    class TimerOC;
    class DmaChannel;
}

namespace ymd::drivers{


class BurstChannelIntf{
public:
    virtual void borrow(std::span<const uint16_t> pbuf);
    virtual void invoke();
    virtual void install();
};

class BurstPwmIntf:public BurstChannelIntf{
public:
    virtual uint32_t calc_cvr_from_duty(const uq32 duty) const;
    virtual uq8 get_period_us() const;

    uint32_t calc_cvr_from_us(const uq8 us){
        return calc_cvr_from_duty(us / get_period_us());
    }
};

class BurstDmaPwm final:public BurstPwmIntf{
public:
    BurstDmaPwm(hal::TimerOC & timer_oc);

    void borrow(std::span<const uint16_t> pbuf);
    void invoke();
    void install();
    bool is_done();
    uint32_t calc_cvr_from_duty(const uq32 duty) const;
    uq8 get_period_us() const;
private:
    hal::TimerOC & timer_oc_;
    hal::DmaChannel & dma_channel_;
    std::span<const uint16_t> pbuf_;
};

class WS2812_PhyIntf{

};

class WS2812_Phy_of_BurstPwm:public WS2812_PhyIntf{
public:
    WS2812_Phy_of_BurstPwm(BurstDmaPwm & burst_dma_pwm);
    void borrow(std::span<const uint16_t> pbuf){
        return burst_dma_pwm_.borrow(pbuf);
    }

    void set_color(std::array<uint8_t, 3> color){
        // apply_color_to_buf()
    }

    bool is_done(){
        return burst_dma_pwm_.is_done();
    }
private:
    std::array<uint16_t, 24> buf_;
    //pure function
    void apply_color_to_buf(std::span<uint16_t, 24> buf, std::array<uint8_t, 3> color) const;
    
    //pure function
    void apply_mono_to_buf(std::span<uint16_t, 8> buf, uint8_t mono) const;

    BurstDmaPwm burst_dma_pwm_;
};


class DShotChannel{
public:
    static constexpr size_t DSHOT_LEN = 40;

    enum class Command : uint8_t {
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


    DShotChannel(hal::TimerOC & oc);
    DShotChannel(const DShotChannel & other) = delete;
    DShotChannel(DShotChannel && other) = delete;

    void init();

    void set_dutycycle(const real_t duty);

private:

    static constexpr uint16_t HIGH_CVR = (234 * 2 / 3);
    static constexpr uint16_t LOW_CVR = (234 * 1 / 3);


    std::array<uint16_t, DSHOT_LEN> buf_ = {0};
    BurstDmaPwm burst_dma_pwm_;

    static uint16_t calculate_crc(uint16_t data_in);

    static void update(std::span<uint16_t, DSHOT_LEN> buf, uint16_t data);

    static void clear(std::span<uint16_t, DSHOT_LEN> buf);

    void invoke();
};

}