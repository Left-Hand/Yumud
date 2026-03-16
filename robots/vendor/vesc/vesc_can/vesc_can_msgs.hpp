#pragma once

#include "vesc_can_primitive.hpp"
#include "vesc_can_utils.hpp"

#include "core/math/float/fp32.hpp"

#include "core/utils/bits/bits_caster.hpp"
#include "primitive/can/bxcan_payload.hpp"


namespace ymd::robots::vesc::can{

// [0] 
// Purpose: Set motor duty cycle (0.0 to 1.0)
// Data Format: [0-3]: Duty cycle as int32 (duty * 100000)
struct [[nodiscard]] alignas(8) SetDuty final{
    using Self = SetDuty;
    static constexpr PacketId PACKET_ID = PacketId::SetDuty;
    static constexpr size_t PAYLOAD_LENGTH = 4;

    Scaled<int32_t, 100000> dutycycle;
};

static_assert(sizeof(SetDuty) == 8);

// [1] 
// Purpose: Set motor current in amperes
// Data Format: [0-3]: Current as int32 (current * 1000)
struct [[nodiscard]] alignas(8) SetCurrent final{
    using Self = SetCurrent;
    static constexpr PacketId PACKET_ID = PacketId::SetCurrent;
    static constexpr size_t PAYLOAD_LENGTH = 4;

    Scaled<int32_t, 1000> current_amps;
};

// [1] (same as SetCurrent but has different dlc)
// Purpose: Set motor current with off delay
// Data Format: [0-3]: Current as int32 (current * 1000), [4-5]: Off delay as int16
struct [[nodiscard]] alignas(8) SetCurrentOffDelay final{
    using Self = SetCurrentOffDelay;
    static constexpr PacketId PACKET_ID = PacketId::SetCurrent;
    static constexpr size_t PAYLOAD_LENGTH = 6;

    Scaled<int32_t, 1000> current_amps;
    Scaled<int16_t, 1> off_delay;
};

// [2]
// Purpose: Set brake current in amperes
// Data Format: [0-3]: Current as int32 (current * 1000)
struct [[nodiscard]] alignas(8) SetCurrentBrake final{
    using Self = SetCurrentBrake;
    static constexpr PacketId PACKET_ID = PacketId::SetCurrentBrake;
    static constexpr size_t PAYLOAD_LENGTH = 4;

    Scaled<int32_t, 1000> current_amps;
};

// [3]
// Purpose: Set motor RPM
// Data Format: [0-3]: RPM as int32
struct [[nodiscard]] alignas(8) SetRpm final{
    using Self = SetRpm;
    static constexpr PacketId PACKET_ID = PacketId::SetRpm;
    static constexpr size_t PAYLOAD_LENGTH = 4;

    int32_t rpm;
};


// [4]
// Purpose: Set motor position
// Data Format: [0-3]: Position as int32 (pos * 100000)
struct [[nodiscard]] alignas(8) SetPos final{
    using Self = SetPos;
    static constexpr PacketId PACKET_ID = PacketId::SetPos;
    static constexpr size_t PAYLOAD_LENGTH = 4;

    Scaled<int32_t, 100000> pos;
};


#if 0
//TODO support buffer

// [5]
// Purpose: Fill receive buffer (7-byte chunks)
struct [[nodiscard]] alignas(8) FillRxBuffer final{
    using Self = FillRxBuffer;
    static constexpr PacketId PACKET_ID = PacketId::FillRxBuffer;
};

// [6]
// Purpose: Fill receive buffer long (6-byte chunks)
struct [[nodiscard]] alignas(8) FillRxBufferLong final{
    using Self = FillRxBufferLong;
    static constexpr PacketId PACKET_ID = PacketId::FillRxBufferLong;
};



// [7]
// Purpose: Process receive buffer
struct [[nodiscard]] alignas(8) ProcessRxBuffer final{
    using Self = ProcessRxBuffer;
    static constexpr PacketId PACKET_ID = PacketId::ProcessRxBuffer;
};

// [8]
// Purpose: Process short buffer (≤6 bytes)
struct [[nodiscard]] alignas(8) ProcessShortBuffer final{
    using Self = ProcessShortBuffer;
    static constexpr PacketId PACKET_ID = PacketId::ProcessShortBuffer;
};

#endif

// [9]
// Purpose: Broadcast motor status (RPM, current, duty)
// Data Format: [0-3]: RPM as int32, [4-5]: Current as int16 (current * 10), [6-7]: Duty as int16 (duty * 1000)
struct [[nodiscard]] alignas(8) Status1 final{
    using Self = Status1;
    static constexpr PacketId PACKET_ID = PacketId::Status1;
    static constexpr size_t PAYLOAD_LENGTH = 8;

    int32_t rpm;
    Scaled<int16_t, 10> current_amps;
    Scaled<int16_t, 1000> dutycycle;
};

static_assert(sizeof(Status1) == 8);

// [10]
// Purpose: Set relative current value, range [-1.0 1.0]
// Data Format: [0-3]: Relative current as int32 (current_rel * 100000)
struct [[nodiscard]] alignas(8) SetCurrentRel final{
    using Self = SetCurrentRel;
    static constexpr PacketId PACKET_ID = PacketId::SetCurrentRel;
    static constexpr size_t PAYLOAD_LENGTH = 4;

    // The relative current value, range [-1.0 1.0]
    Scaled<int32_t, 100000> current_rel;
};

// [10] (same as SetCurrentRel but has different dlc)
// Purpose: Set relative current with off delay
// Data Format: [0-3]: Relative current as float32 (scale 1e5), [4-5]: Off delay as float16 (scale 1e3)
struct [[nodiscard]] alignas(8) SetCurrentRelOffDelay final{
    using Self = SetCurrentRelOffDelay;
    static constexpr PacketId PACKET_ID = PacketId::SetCurrentRel;
    static constexpr size_t PAYLOAD_LENGTH = 6;

    Scaled<int32_t, 100000> current_rel;
    Scaled<int16_t, 1000> off_delay;
};


// [11]
// Purpose: Set relative brake current value, range [-1.0 1.0]
// Data Format: [0-3]: Relative brake current as int32 (current_rel * 100000)
struct [[nodiscard]] alignas(8) SetCurrentBrakeRel final{
    using Self = SetCurrentBrakeRel;
    static constexpr PacketId PACKET_ID = PacketId::SetCurrentBrakeRel;
    static constexpr size_t PAYLOAD_LENGTH = 4;

    // The relative current value, range [-1.0 1.0]
    Scaled<int32_t, 100000> current_rel;
};


// [12]
// Purpose: Set handbrake current in amperes
// Data Format: [0-3]: Current as int32 (current * 1000)
struct [[nodiscard]] alignas(8) SetCurrentHandbrake final{
    using Self = SetCurrentHandbrake;
    static constexpr PacketId PACKET_ID = PacketId::SetCurrentHandbrake;
    static constexpr size_t PAYLOAD_LENGTH = 4;

    Scaled<int32_t, 1000> current_amps;
};


// [13]
// Purpose: Set relative handbrake current value, range [-1.0 1.0]
// Data Format: [0-3]: Relative current as int32 (current_rel * 100000)
struct [[nodiscard]] alignas(8) SetCurrentHandbrakeRel final{
    using Self = SetCurrentHandbrakeRel;
    static constexpr PacketId PACKET_ID = PacketId::SetCurrentHandbrakeRel;
    static constexpr size_t PAYLOAD_LENGTH = 4;

    // The relative current value, range [-1.0 1.0]
    Scaled<int32_t, 100000> current_rel;
};


// [14]
// Purpose: Broadcast amp hours consumed and charged
// Data Format: [0-3]: Ah consumed as int32 (Ah * 10000), [4-7]: Ah charged as int32 (Ah * 10000)
struct [[nodiscard]] alignas(8) Status2 final{
    using Self = Status2;
    static constexpr PacketId PACKET_ID = PacketId::Status2;
    static constexpr size_t PAYLOAD_LENGTH = 8;

    Scaled<int32_t, 10000> ah_consumed;
    Scaled<int32_t, 10000> ah_charged;
};


// [15]
// Purpose: Broadcast watt hours consumed and charged
// Data Format: [0-3]: Wh consumed as int32 (Wh * 10000), [4-7]: Wh charged as int32 (Wh * 10000)
struct [[nodiscard]] alignas(8) Status3 final{
    using Self = Status3;
    static constexpr PacketId PACKET_ID = PacketId::Status3;
    static constexpr size_t PAYLOAD_LENGTH = 8;

    //  watt hours drawn from the input source
    Scaled<int32_t, 10000> wh_consumed;
    Scaled<int32_t, 10000> wh_charged;
};


// [16]
// Purpose: Broadcast temperatures, input current, and PID position
// Data Format: [0-1]: FET temp as int16 (temp * 10), [2-3]: Motor temp as int16 (temp * 10),
//              [4-5]: Input current as int16 (current * 10), [6-7]: PID pos as int16 (pos * 50)
struct [[nodiscard]] alignas(8) Status4 final{
    using Self = Status4;
    static constexpr PacketId PACKET_ID = PacketId::Status4;
    static constexpr size_t PAYLOAD_LENGTH = 8;

    Scaled<int16_t, 10> fet_temp;
    Scaled<int16_t, 10> motor_temp;
    Scaled<int16_t, 10> input_current;
    Scaled<int16_t, 50> pid_pos;
};

static_assert(sizeof(Status4) == 8);


// [17]
// Purpose: Ping request
struct [[nodiscard]] alignas(8) Ping final{
    using Self = Ping;
    static constexpr PacketId PACKET_ID = PacketId::Ping;

    static constexpr size_t PAYLOAD_LENGTH = 1;

    uint8_t controller_id;

};


// [18]
// Purpose: Ping response
struct [[nodiscard]] alignas(8) Pong final{
    using Self = Pong;

    enum class HwType:uint8_t { 
        VESC = 0,
        VESC_BMS,
        CUSTOM_MODULE
    };

    static constexpr PacketId PACKET_ID = PacketId::Pong;

    static constexpr size_t PAYLOAD_LENGTH = 2;

    uint8_t motor_id;

    constexpr void fill_bytes(std::span<uint8_t, PAYLOAD_LENGTH> bytes) const{
        bytes[0] = motor_id;
        bytes[1] = static_cast<uint8_t>(HwType::VESC);
    }
};


// [19]
// Purpose: Detect and apply all FOC
struct [[nodiscard]] alignas(8) DetectApplyAllFoc final{
    using Self = DetectApplyAllFoc;
    static constexpr PacketId PACKET_ID = PacketId::DetectApplyAllFoc;

    static constexpr size_t PAYLOAD_LENGTH = 6;

    uint8_t controller_id;
    bool activate_status_msgs;
    Scaled<int32_t, 1000> max_power_loss;
};

static_assert(sizeof(DetectApplyAllFoc) == 8);

// [20]
// Purpose: Detect and apply all FOC result
struct [[nodiscard]] alignas(8) DetectApplyAllFocRes final{
    using Self = DetectApplyAllFocRes;
    static constexpr PacketId PACKET_ID = PacketId::DetectApplyAllFocRes;

    static constexpr size_t PAYLOAD_LENGTH = 1;

    //res is the invoke result from 'conf_general_detect_apply_all_foc'
    uint8_t res;
};


// [21]
// Purpose: Configure current limits
struct [[nodiscard]] alignas(8) ConfCurrentLimits final{
    using Self = ConfCurrentLimits;
    static constexpr PacketId PACKET_ID = PacketId::ConfCurrentLimits;

    static constexpr size_t PAYLOAD_LENGTH = 8;

    Scaled<int32_t, 1000> min;
    Scaled<int32_t, 1000> max;
};


// [22]
// Purpose: Store current limits
struct [[nodiscard]] alignas(8) ConfStoreCurrentLimits final{
    using Self = ConfStoreCurrentLimits;
    static constexpr PacketId PACKET_ID = PacketId::ConfStoreCurrentLimits;

    static constexpr size_t PAYLOAD_LENGTH = 8;

    Scaled<int32_t, 1000> min;
    Scaled<int32_t, 1000> max;
};


// [23]
// Purpose: Configure input current limits
struct [[nodiscard]] alignas(8) ConfCurrentLimitsIn final{
    using Self = ConfCurrentLimitsIn;
    static constexpr PacketId PACKET_ID = PacketId::ConfCurrentLimitsIn;

    static constexpr size_t PAYLOAD_LENGTH = 8;

    Scaled<int32_t, 1000> min;
    Scaled<int32_t, 1000> max;
};


// [24]
// Purpose: Store input current limits
struct [[nodiscard]] alignas(8) ConfStoreCurrentLimitsIn final{
    using Self = ConfStoreCurrentLimitsIn;
    static constexpr PacketId PACKET_ID = PacketId::ConfStoreCurrentLimitsIn;

    static constexpr size_t PAYLOAD_LENGTH = 8;

    Scaled<int32_t, 1000> min;
    Scaled<int32_t, 1000> max;
};


// [25]
// Purpose: Configure FOC ERPMs
struct [[nodiscard]] alignas(8) ConfFocErpms final{
    using Self = ConfFocErpms;
    static constexpr PacketId PACKET_ID = PacketId::ConfFocErpms;

    static constexpr size_t PAYLOAD_LENGTH = 8;

    Scaled<int32_t, 1000> foc_openloop_rpm;
    Scaled<int32_t, 1000> foc_sl_rpm;
};


// [26]
// Purpose: Store FOC ERPMs
struct [[nodiscard]] alignas(8) ConfStoreFocErpms final{
    using Self = ConfStoreFocErpms;
    static constexpr PacketId PACKET_ID = PacketId::ConfStoreFocErpms;

    static constexpr size_t PAYLOAD_LENGTH = 8;

    Scaled<int32_t, 1000> foc_openloop_rpm;
    Scaled<int32_t, 1000> foc_sl_rpm;
};


// [27]
// Purpose: Status message 5 (Tachometer, input voltage)
struct [[nodiscard]] alignas(8) Status5 final{
    using Self = Status5;
    static constexpr PacketId PACKET_ID = PacketId::Status5;
    static constexpr size_t PAYLOAD_LENGTH = 8;

    int32_t tacho_value;
    Scaled<int16_t, 10> v_in_filtered;

    constexpr void fill_bytes(const std::span<uint8_t, PAYLOAD_LENGTH> bytes) const {
        auto & self = *this;
        auto filler = BytesFiller(bytes);
        filler.push_be_i32(self.tacho_value);
        filler.push_be_i16(self.v_in_filtered.bits);
        filler.push_be_i16(0);
    }
};


// [28]
// Purpose: Poll TS5700N8501 status
struct [[nodiscard]] alignas(8) PollTS5700N8501Status final{
    using Self = PollTS5700N8501Status;
    static constexpr PacketId PACKET_ID = PacketId::PollTS5700N8501Status;

    TS5700N8501Status status;

    constexpr void fill_bytes(std::span<uint8_t, 8> bytes) const{
        const auto status_bytes = status.as_bytes();
        std::copy(status_bytes.begin(), status_bytes.end(), bytes.begin());
    }
};

static_assert(sizeof(PollTS5700N8501Status) == 8);


// [29]
// Purpose: Configure battery cut
struct [[nodiscard]] alignas(8) ConfBatteryCut final{
    using Self = ConfBatteryCut;
    static constexpr PacketId PACKET_ID = PacketId::ConfBatteryCut;

    static constexpr size_t PAYLOAD_LENGTH = 8;

    Scaled<int32_t, 1000> start;
    Scaled<int32_t, 1000> stop;
};


// [30]
// Purpose: Store battery cut
struct [[nodiscard]] alignas(8) ConfStoreBatteryCut final{
    using Self = ConfStoreBatteryCut;
    static constexpr PacketId PACKET_ID = PacketId::ConfStoreBatteryCut;

    static constexpr size_t PAYLOAD_LENGTH = 8;

    Scaled<int32_t, 1000> start;
    Scaled<int32_t, 1000> stop;
};


// [31]
// Purpose: Shutdown
struct [[nodiscard]] Shutdown final{
    using Self = Shutdown;
    static constexpr PacketId PACKET_ID = PacketId::Shutdown;

    static constexpr size_t PAYLOAD_LENGTH = 0;
};


// [32]
// Purpose: IO board ADC 1-4
// Data Format: Multiple float16 values (scale 1e2)
struct [[nodiscard]] alignas(8) IoBoardAdc1To4 final{
    using Self = IoBoardAdc1To4;
    static constexpr PacketId PACKET_ID = PacketId::IoBoardAdc1To4;
    // Variable length: 2 bytes per ADC value

    std::array<Scaled<int16_t, 100>, 4> adc_voltages;
};


// [33]
// Purpose: IO board ADC 5-8
// Data Format: Multiple float16 values (scale 1e2)
struct [[nodiscard]] alignas(8) IoBoardAdc5To8 final{
    using Self = IoBoardAdc5To8;
    static constexpr PacketId PACKET_ID = PacketId::IoBoardAdc5To8;
    // Variable length: 2 bytes per ADC value

    std::array<Scaled<int16_t, 100>, 4> adc_voltages;
};


// [34]
// Purpose: IO board ADC 9-12
// Data Format: Multiple float16 values (scale 1e2)
struct [[nodiscard]] alignas(8) IoBoardAdc9To12 final{
    using Self = IoBoardAdc9To12;
    static constexpr PacketId PACKET_ID = PacketId::IoBoardAdc9To12;
    // Variable length: 2 bytes per ADC value

    std::array<Scaled<int16_t, 100>, 4> adc_voltages;
};


// [35]
// Purpose: IO board digital inputs
// Data Format: Bitmask of digital inputs as uint64 (spread over multiple bytes)
struct [[nodiscard]] alignas(8) IoBoardDigitalIn final{
    using Self = IoBoardDigitalIn;
    static constexpr PacketId PACKET_ID = PacketId::IoBoardDigitalIn;
    // Variable length: up to 8 bytes for 64-bit bitmask

    uint64_t inputs;
};


// [36]
// Purpose: Set IO board digital outputs
// Data Format: [0]: Channel, [1]: Enable flag, [2]: On/off state
struct [[nodiscard]] alignas(8) IoBoardSetOutputDigital final{
    using Self = IoBoardSetOutputDigital;
    static constexpr PacketId PACKET_ID = PacketId::IoBoardSetOutputDigital;
    static constexpr size_t PAYLOAD_LENGTH = 3;

    uint8_t channel;
    uint8_t on_state;

    constexpr void fill_bytes(const std::span<uint8_t, PAYLOAD_LENGTH> bytes) const {
        bytes[0] = channel;
        bytes[1] = 1;
        bytes[2] = on_state;
    }
};


// [37]
// Purpose: Set IO board PWM outputs
// Data Format: [0]: Channel, [1-2]: Duty cycle as float16 (scale 1e3)
struct [[nodiscard]] alignas(8) IoBoardSetOutputPwm final{
    using Self = IoBoardSetOutputPwm;
    static constexpr PacketId PACKET_ID = PacketId::IoBoardSetOutputPwm;
    static constexpr size_t PAYLOAD_LENGTH = 3;

    uint8_t channel;
    Scaled<int16_t, 1000> duty;
};


// [38]
// Purpose: BMS total voltage
struct [[nodiscard]] alignas(8) BmsVTot final{
    using Self = BmsVTot;
    static constexpr PacketId PACKET_ID = PacketId::BmsVTot;
    static constexpr size_t PAYLOAD_LENGTH = 8;
    
    math::fp32 v_tot;
    math::fp32 v_charge;
};


// [39]
// Purpose: BMS current
struct [[nodiscard]] alignas(8) BmsI final{
    using Self = BmsI;
    static constexpr PacketId PACKET_ID = PacketId::BmsI;
    static constexpr size_t PAYLOAD_LENGTH = 8;
    
    math::fp32 i_in;
    math::fp32 i_in_ic;
};


// [40]
// Purpose: BMS amp hours and watt hours
struct [[nodiscard]] alignas(8) BmsAhWh final{
    using Self = BmsAhWh;
    static constexpr PacketId PACKET_ID = PacketId::BmsAhWh;
    static constexpr size_t PAYLOAD_LENGTH = 8;
    
    math::fp32 i_in;
    math::fp32 i_in_ic;
};


// [41]
// Purpose: BMS cell voltages
struct [[nodiscard]] alignas(8) BmsVCell final{
    using Self = BmsVCell;
    static constexpr PacketId PACKET_ID = PacketId::BmsVCell;

    //TODO 参考[43]
};


// [42]
// Purpose: BMS balancing
struct [[nodiscard]] alignas(8) BmsBal final{
    using Self = BmsBal;
    static constexpr PacketId PACKET_ID = PacketId::BmsBal;
    static constexpr size_t PAYLOAD_LENGTH = 8;

    U64Bitset bal_state;
};

static_assert(sizeof(BmsBal) == 8);


// [43]
// Purpose: BMS temperatures
struct [[nodiscard]] alignas(8) BmsTemps final{
    using Self = BmsTemps;
    static constexpr PacketId PACKET_ID = PacketId::BmsTemps;

    #if 0
    // 每个包格式：
    // uint8_t temp_now;
    // uint8_t temp_max;
    // std::array<Scaled<int16_t, 100> | Null, 3> temps;

    template<typename T>
    constexpr hal::ClassicCanPayload to_payloads(
        uint8_t temp_now,
        uint8_t temp_max,
        std::span<const T> temps
    ){
        std::array<uint8_t, 8> buffer;
		buffer[0] = temp_now;
		buffer[1] = temp_max;
		size_t len = 2;

        auto try_push = [&]() -> bool {
            if (temp_now < temp_max) {
                const int16_t bits = static_cast<int16_t>(temp_now * 100u);
                buffer[len++] = static_cast<uint8_t>(bits >> 8);
                buffer[len++] = static_cast<uint8_t>(bits );
                temp_now++;
            }
		};

        try_push();
        try_push();
        try_push();

        co_yield hal::ClassicCanPayload::from_bytes(std::span(buffer.data(), len));
    }
    #endif
};


// [44]
// Purpose: BMS humidity
struct [[nodiscard]] alignas(8) BmsHum final{
    using Self = BmsHum;
    static constexpr PacketId PACKET_ID = PacketId::BmsHum;

    Scaled<int16_t, 100> temp_hum;
    Scaled<int16_t, 100> hum;
    Scaled<int16_t, 100> temp_ic;
    InvScaled<int16_t, 100> pressure;
};


// [45]
// Purpose: BMS SOC, SOH, temperature status
struct [[nodiscard]] alignas(8) BmsSocSohTempStat final{
    using Self = BmsSocSohTempStat;
    static constexpr PacketId PACKET_ID = PacketId::BmsSocSohTempStat;

    Scaled<int16_t, 10u> v_cell_min;
    Scaled<int16_t, 10u> v_cell_max;

    uint8_t soc;
    uint8_t soh;
    int8_t temp_max_cell;

    struct StateBitFields{
        uint8_t is_chg:1;
        uint8_t is_bal:1;
        uint8_t chg_ok:1;
        uint8_t __resv__:1;
        uint8_t data_version:4;
    };  

    StateBitFields state;

};

static_assert(sizeof(BmsSocSohTempStat) == 8);


// [46]
// Purpose: Power switch status
// Data Format: [0-1]: V_in as float16 (scale 10.0), [2-3]: V_out as float16 (scale 10.0), 
//              [4-5]: Temp as float16 (scale 10.0), [6]: Status flags
struct [[nodiscard]] alignas(8) PswStat final{
    using Self = PswStat;
    static constexpr PacketId PACKET_ID = PacketId::PswStat;
    static constexpr size_t PAYLOAD_LENGTH = 7;

    Scaled<int16_t, 10> v_in;
    Scaled<int16_t, 10> v_out;
    Scaled<int16_t, 10> temp;
    uint8_t is_out_on : 1;
    uint8_t is_pch_on : 1;
    uint8_t is_dsc_on : 1;
    uint8_t __resv__ : 5;
};


static_assert(sizeof(PswStat) == 8);

// [47]
// Purpose: Power switch control
// Data Format: [0]: Is on flag, [1]: Plot flag
struct [[nodiscard]] alignas(8) PswSwitch final{
    using Self = PswSwitch;
    static constexpr PacketId PACKET_ID = PacketId::PswSwitch;
    static constexpr size_t PAYLOAD_LENGTH = 2;

    uint8_t is_on;
    uint8_t plot;
};


// [48-54]
// Purpose: BMS hardware data packets
struct [[nodiscard]] alignas(8) BmsHwData1 final{
    using Self = BmsHwData1;
    static constexpr PacketId PACKET_ID = PacketId::BmsHwData1;
    static constexpr size_t PAYLOAD_LENGTH = 8;

    std::array<uint8_t, PAYLOAD_LENGTH> status;
};


struct [[nodiscard]] alignas(8) BmsHwData2 final{
    using Self = BmsHwData2;
    static constexpr PacketId PACKET_ID = PacketId::BmsHwData2;
    static constexpr size_t PAYLOAD_LENGTH = 8;

    std::array<uint8_t, PAYLOAD_LENGTH> status;
};


struct [[nodiscard]] alignas(8) BmsHwData3 final{
    using Self = BmsHwData3;
    static constexpr PacketId PACKET_ID = PacketId::BmsHwData3;
    static constexpr size_t PAYLOAD_LENGTH = 8;

    std::array<uint8_t, PAYLOAD_LENGTH> status;
};


struct [[nodiscard]] alignas(8) BmsHwData4 final{
    using Self = BmsHwData4;
    static constexpr PacketId PACKET_ID = PacketId::BmsHwData4;
    static constexpr size_t PAYLOAD_LENGTH = 8;

    std::array<uint8_t, PAYLOAD_LENGTH> status;
};


struct [[nodiscard]] alignas(8) BmsHwData5 final{
    using Self = BmsHwData5;
    static constexpr PacketId PACKET_ID = PacketId::BmsHwData5;
    static constexpr size_t PAYLOAD_LENGTH = 8;

    std::array<uint8_t, PAYLOAD_LENGTH> status;
};


// [53]
// Purpose: BMS total charged amp hours and watt hours
struct [[nodiscard]] alignas(8) BmsAhWhChgTotal final{
    using Self = BmsAhWhChgTotal;
    static constexpr PacketId PACKET_ID = PacketId::BmsAhWhChgTotal;
    static constexpr size_t PAYLOAD_LENGTH = 8;

    math::fp32 ah_cnt_chg_total;
    math::fp32 wh_cnt_chg_total;
};


// [54]
// Purpose: BMS total discharged amp hours and watt hours
struct [[nodiscard]] alignas(8) BmsAhWhDisTotal final{
    using Self = BmsAhWhDisTotal;
    static constexpr PacketId PACKET_ID = PacketId::BmsAhWhDisTotal;
    static constexpr size_t PAYLOAD_LENGTH = 8;

    math::fp32 ah_cnt_dis_total;
    math::fp32 wh_cnt_dis_total;
};


// [55]
// Purpose: Update PID position offset
// Data Format: [0-3]: Angle now as float32 (scale 1e4), [4]: Store flag
struct [[nodiscard]] alignas(8) UpdatePidPosOffset final{
    using Self = UpdatePidPosOffset;
    static constexpr PacketId PACKET_ID = PacketId::UpdatePidPosOffset;
    static constexpr size_t PAYLOAD_LENGTH = 5;

    Scaled<int32_t, 10000> angle_now;
    uint8_t store;
};


// [56]
// Purpose: Poll rotor position
// Data Format: [0-3]: Position as int32 (scale 1e5)
struct [[nodiscard]] alignas(8) PollRotorPos final{
    using Self = PollRotorPos;
    static constexpr PacketId PACKET_ID = PacketId::PollRotorPos;
    static constexpr size_t PAYLOAD_LENGTH = 4;

    Scaled<int32_t, 100000> position;
};


// [57]
// Purpose: Notify bootloader
struct [[nodiscard]] alignas(8) NotifyBoot final{
    using Self = NotifyBoot;
    static constexpr PacketId PACKET_ID = PacketId::NotifyBoot;
};


// [58]
// Purpose: Status message 6 (ADC values, PPM)
// Data Format: [0-1]: ADC1 as float16 (scale 1e3), [2-3]: ADC2 as float16 (scale 1e3),
//              [4-5]: ADC3 as float16 (scale 1e3), [6-7]: PPM as float16 (scale 1e3)
struct [[nodiscard]] alignas(8) Status6 final{
    using Self = Status6;
    static constexpr PacketId PACKET_ID = PacketId::Status6;
    static constexpr size_t PAYLOAD_LENGTH = 8;

    Scaled<int16_t, 1000> adc_1;
    Scaled<int16_t, 1000> adc_2;
    Scaled<int16_t, 1000> adc_3;

    // The servo value in the range
    Scaled<int16_t, 1000> ppm;
};


// [59]
// Purpose: GNSS time
// Data Format: [0-3]: ms_today as int32, [4-5]: Year as int16, [6]: Month, [7]: Day
struct [[nodiscard]] alignas(8) GnssTime final{
    using Self = GnssTime;
    static constexpr PacketId PACKET_ID = PacketId::GnssTime;
    static constexpr size_t PAYLOAD_LENGTH = 8;

    int32_t ms_today;
    int16_t yy;
    uint8_t mo;
    uint8_t dd;
};

static_assert(sizeof(GnssTime) == 8);


// [60]
// Purpose: GNSS latitude
// Data Format: [0-7]: Latitude as double64 (scale 1e16)
struct [[nodiscard]] alignas(8) GnssLat final{
    using Self = GnssLat;
    static constexpr PacketId PACKET_ID = PacketId::GnssLat;
    static constexpr size_t PAYLOAD_LENGTH = 8;

    double lat;
};


// [61]
// Purpose: GNSS longitude
// Data Format: [0-7]: Longitude as double64 (scale 1e16)
struct [[nodiscard]] alignas(8) GnssLon final{
    using Self = GnssLon;
    static constexpr PacketId PACKET_ID = PacketId::GnssLon;
    static constexpr size_t PAYLOAD_LENGTH = 8;

    double lon;
};


// [62]
// Purpose: GNSS altitude, speed, HDOP
// Data Format: [0-3]: Height as float32 auto, [4-5]: Speed as float16 (scale 1e2), [6-7]: HDOP as float16 (scale 1e2)
struct [[nodiscard]] alignas(8) GnssAltSpeedHdop final{
    using Self = GnssAltSpeedHdop;
    static constexpr PacketId PACKET_ID = PacketId::GnssAltSpeedHdop;
    static constexpr size_t PAYLOAD_LENGTH = 8;

    Scaled<int16_t, 1> height;
    Scaled<int16_t, 100> speed;
    Scaled<int16_t, 100> hdop;
};

// [63]
struct [[nodiscard]] alignas(8) UpdateBaud final{
    using Self = UpdateBaud;
    static constexpr PacketId PACKET_ID = PacketId::UpdateBaud;
    static constexpr size_t PAYLOAD_LENGTH = 4;

    uint16_t kbits;
    uint16_t delay_msec;
};


} // namespace ymd::robots::vesc::can
