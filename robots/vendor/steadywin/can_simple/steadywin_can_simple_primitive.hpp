#pragma once

#include "../steadywin_primitive.hpp"
#include "primitive/can/bxcan_frame.hpp"

// 伺泰威对Odrive通讯消息进行了魔改 不能直接等效于Odrive
// 参考Odrive的源码，所有Can报文的数据载荷都是8字节

// 参考代码
//      https://docs.rs/odrive-cansimple/latest/src/odrive_cansimple/enumerations/axis_error.rs.html

namespace ymd::robots::steadywin::can_simple{

struct [[nodiscard]] FrameId final{
    AxisId axis_id;
    Command command;

    static constexpr FrameId from_stdid(const hal::CanStdId & stdid){
        const auto id_u11 = stdid.to_u11();
        return {
            AxisId::from_bits(static_cast<uint8_t>((id_u11 >> 5) & 0b111111)),
            Command::from_bits(id_u11 & 0b11111)
        };
    }

    constexpr hal::CanStdId to_stdid() const { 
        return hal::CanStdId::from_u11(
            axis_id.to_b6().connect(command.to_b5()).to_bits()
        );
    }  

    friend OutputStream & operator<<(OutputStream & os, const FrameId & primitive) {
        return os << os.field("axis_id")(primitive.axis_id) << os.splitter() 
            << os.field("command")(primitive.command);
    }
};

}
