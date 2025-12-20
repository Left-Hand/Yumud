#include "mks_stepper_primitive.hpp"

namespace ymd::robots::mksmotor{

namespace req_msgs{


struct [[nodiscard]] SetPositionMode3 final{
    static constexpr FuncCode FUNC_CODE = FuncCode::PositionCtrl3;
    Rpm rpm;
    AcclerationLevel acc_level;
    PulseCnt abs_pulse_cnt;
};

struct [[nodiscard]] StopPositionMode3 final{
    static constexpr FuncCode FUNC_CODE = FuncCode::PositionCtrl3;

    const Rpm rpm;
    AcclerationLevel acc_level;
    const PulseCnt abs_pulse_cnt;
};

struct [[nodiscard]] SetSpeed final{
    static constexpr FuncCode FUNC_CODE = FuncCode::SpeedCtrl;
    iRpm rpm;
    AcclerationLevel acc_level;
};

struct [[nodiscard]] SetEnableStatus final{
    static constexpr FuncCode FUNC_CODE = FuncCode::SetEnableStatus;
    bool is_enabled;
};


struct [[nodiscard]] SetSubdivides final{
    static constexpr FuncCode FUNC_CODE = FuncCode::SetSubdivides;
    uint8_t subdivides;
};

struct [[nodiscard]] SetEndstopParaments final{
    static constexpr FuncCode FUNC_CODE = FuncCode::SetEndstopParaments;
    bool is_high;
    bool is_ccw;
    Rpm rpm;
};

struct [[nodiscard]] EndstopHomming final{
    static constexpr FuncCode FUNC_CODE = FuncCode::EndstopHomming;
};

template<typename Raw, typename T = std::decay_t<Raw>>
static std::span<const uint8_t> serialize(
    Raw && obj
){
    return std::span(
        reinterpret_cast<const uint8_t *>(&obj),
        tmp::pure_sizeof_v<T>
    );
}

}
}