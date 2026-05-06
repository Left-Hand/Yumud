#pragma once

#include "stl06n_utils.hpp"

namespace ymd::drivers::stl06n{

struct [[nodiscard]] ReqArg final{
    using Self = ReqArg;

    uint32_t bits;

    static constexpr Self zero(){
        return Self{0};
    }
};

struct req_msgs{
struct [[nodiscard]] Start final{
    static constexpr Command COMMAND = Command::Start;

    constexpr ReqArg to_req_arg() const noexcept {
        return ReqArg::zero();
    }
};

struct [[nodiscard]] Stop final{
    static constexpr Command COMMAND = Command::Stop;
    constexpr ReqArg to_req_arg() const noexcept {
        return ReqArg::zero();
    }
};

struct [[nodiscard]] SetSpeed final{
    static constexpr Command COMMAND = Command::SetSpeed;

    LidarSpinSpeedCode speed;
    constexpr ReqArg to_req_arg() const noexcept {
        return ReqArg{static_cast<uint32_t>(speed.bits)};
    }
};

struct [[nodiscard]] GetSpeed final{
    static constexpr Command COMMAND = Command::GetSpeed;

    constexpr ReqArg to_req_arg() const noexcept {
        return ReqArg::zero();
    }
};
};

struct resp_msgs{
struct [[nodiscard]] Start final{
    static constexpr Command COMMAND = Command::Start;

};

struct [[nodiscard]] Stop final{
    static constexpr Command COMMAND = Command::Stop;
};

struct [[nodiscard]] SetSpeed final{
    static constexpr Command COMMAND = Command::SetSpeed;
};

struct [[nodiscard]] GetSpeed final{
    static constexpr Command COMMAND = Command::GetSpeed;
    LidarSpinSpeedCode speed;
};
};

namespace events{
struct [[nodiscard]] DataReady final{
    const LidarSectorPacket & sector;
};

struct [[nodiscard]] InvalidCrc final{
    using Self = InvalidCrc;
    Command command;
    uint8_t expected;
    uint8_t actual;

    friend OutputStream & operator <<(OutputStream & os, const Self & self) noexcept {
        return os << os.field("command")(self.command) << os.splitter()
            << os.field("expected")(self.expected) << os.splitter()
            << os.field("actual")(self.actual);
    }
};

using Start = resp_msgs::Start;
using Stop = resp_msgs::Stop;
using SetSpeed = resp_msgs::SetSpeed;
using GetSpeed = resp_msgs::GetSpeed;
};

struct Event:public Sumtype<
    events::DataReady,
    events::InvalidCrc,
    events::Start,
    events::Stop,
    events::SetSpeed,
    events::GetSpeed
    > {
    using DataReady = events::DataReady;
    using InvalidCrc = events::InvalidCrc;


    using Start = events::Start;
    using Stop = events::Stop;
    using SetSpeed = events::SetSpeed;
    using GetSpeed = events::GetSpeed;
};


using Callback = std::function<void(Event)>;
}