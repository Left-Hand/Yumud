#pragma once

#include "asciican_utils.hpp"
#include "hal/bus/can/can.hpp"

namespace ymd::asciican{
using namespace ymd::asciican::primitive;


class AsciiCanPhy final{
public:
    using Msg = hal::CanClassicFrame;
    using Error = asciican::Error;

    template<typename T = void>
    using IResult = Result<T, Error>;

public:
    AsciiCanPhy(hal::Can & can):
        can_(can){;}

    [[nodiscard]] IResult<> send_can_frame(const Msg && msg);

    [[nodiscard]] IResult<> send_str(const StringView str);

    [[nodiscard]] IResult<> set_stream_baud(const uint32_t baud);

    [[nodiscard]] IResult<> set_can_baud(const uint32_t baud);

    [[nodiscard]] IResult<> open();

    [[nodiscard]] IResult<> close();



    hal::Can & can_;
};
}