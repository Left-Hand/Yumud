#pragma once

#include "core/math/real.hpp"
#include "core/math/float/bf16.hpp"


namespace ymd::nuedc_2025e{
static constexpr size_t MACHINE_CTRL_FREQ = 200;
static constexpr auto DELTA_TIME_MS = 1000ms / MACHINE_CTRL_FREQ;
static constexpr auto DELTA_TIME = DELTA_TIME_MS.count() * 0.001_q20;

static constexpr q20 PITCH_JOINT_POSITION_LIMIT = 0.2_r;
static constexpr size_t CANMSG_QUEUE_SIZE = 8;

static constexpr auto MAX_STATIC_SHOT_ERR = 0.007_q20; 
static constexpr auto PITCH_SEEKING_ANGLE = 0.012_q20;
static constexpr q20 PITCH_MAX_POSITION = 0.06_r;
static constexpr q20 PITCH_MIN_POSITION = -0.03_r;

static constexpr auto ADVANCED_BLINK_PERIOD_MS = 7000ms;
static constexpr auto STATIC_SHOT_FIRE_MS = 180ms;

static constexpr auto GEN2_TIMEOUT = 1700ms;
static constexpr auto GEN3_TIMEOUT = 3700ms;


using Vector2u8 = Vec2<uint8_t>;
using Vector2q20 = Vec2<q20>;



//CTAD


enum class RunState:uint8_t{
    Idle,
    Seeking,
    Tracking
};

DEF_DERIVE_DEBUG(RunState)


struct RunStatus{
    using State = RunState;
    RunState state = RunState::Idle;
};


}



#define DEF_COMMAND_BIND(K, T) \
template<> \
struct command_to_kind<CommandKind, T>{ \
    static constexpr CommandKind KIND = K; \
};\
template<> \
struct kind_to_command<CommandKind, K>{ \
    using type = T; \
};


#define DEF_QUICK_COMMAND_BIND(NAME) DEF_COMMAND_BIND(CommandKind::NAME, commands::NAME)

namespace ymd{



enum class CommandKind:uint8_t{
    ResetNode,
    SetPosition,
    SetPositionWithFwdSpeed,
    SetSpeed,
    SetKpKd,
    // Deactivate,
    // Activate,
    // PerspectiveRectInfo,
    StartSeeking,
    StartTracking,
    StopTracking,
    DeltaPosition,
    ErrXY,
    FwdXY
};


namespace commands{ 
    using namespace robots::joint_commands;
    using namespace robots::nmt_commands;
}


struct ErrPosition{
    // std::array<Vec2<uint8_t>, 4> points;
    bf16 px;
    bf16 py;
    bf16 z;
    bf16 e;

    friend OutputStream & operator << (OutputStream & os, const ErrPosition self){
        return os << os.brackets<'('>() 
            << self.px << os.splitter() 
            << self.py << os.splitter() 
            << self.z << os.splitter() 
            << self.e << os.splitter() 
        << os.brackets<')'>();
    }
};



DEF_QUICK_COMMAND_BIND(ResetNode)
DEF_QUICK_COMMAND_BIND(SetPosition)
DEF_QUICK_COMMAND_BIND(SetPositionWithFwdSpeed)
DEF_QUICK_COMMAND_BIND(SetSpeed)
DEF_QUICK_COMMAND_BIND(SetKpKd)
DEF_QUICK_COMMAND_BIND(StartSeeking)
DEF_QUICK_COMMAND_BIND(StartTracking)
DEF_QUICK_COMMAND_BIND(StopTracking)
DEF_QUICK_COMMAND_BIND(DeltaPosition)
DEF_QUICK_COMMAND_BIND(ErrXY)
DEF_QUICK_COMMAND_BIND(FwdXY)

}