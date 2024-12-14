#pragma once

#include "sys/math/real.hpp"

namespace ymd{
    class TimerOC;
    class DmaChannel;
}

namespace ymd::drivers{

class DShotChannel{
public:
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

    uint16_t buf[40] = {0};
protected:
    uint16_t high_cnt;
    uint16_t low_cnt;
    TimerOC & oc;
    DmaChannel & dma_channel;

    // bool enabled = false;

    scexpr uint16_t m_crc(uint16_t data_in){
        uint16_t speed_data;
        speed_data = data_in << 5;
        data_in = data_in << 1;
        data_in = (data_in ^ (data_in >> 4) ^ (data_in >> 8)) & 0x0f;
        return speed_data | data_in;
    }
    
    void update(uint16_t data);
    void invoke();
public:
    DShotChannel(TimerOC & _oc);
    DShotChannel(DShotChannel & other) = delete;
    DShotChannel(DShotChannel && other) = delete;

    void init();

    auto & operator = (const real_t duty){
        // DEBUG_PRINTLN(duty);
        if(duty != 0) update(m_crc(MAX(int(duty * 2047), 48)));
        else update(0);
        // DEBUG_PRINTLN(buf);
        invoke();
        return *this;
    }
};

}