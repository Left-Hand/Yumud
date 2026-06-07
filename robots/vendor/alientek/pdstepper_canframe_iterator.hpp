#pragma once

#include "primitive/can/can_frame.hpp"
#include "core/utils/scope_guard.hpp"


// 在 CAN 总线上使用本协议时，需要注意以下几点：
//  - 仅支持扩展帧 (Extended Frame)。
//  - 报文 ID 规范：ID 必须符合 0x100x 格式，其中最后的 x 表示任意 16 进制数。
// 例如：0x1001、0x100A、0x100F 均为合法 ID。
//  - 除上述限制外，协议内容（指令、数据格式、校验规则等）与 串口 TTL/RS485 版
// 本保持完全一致
namespace ymd::robots::pdstepper{

struct [[nodiscard]] CanframeSlicingIterator final{
    constexpr CanframeSlicingIterator(
        const uint8_t node_id, 
        std::span<const uint8_t> bytes
    ):
        can_id_(hal::CanExtId::from_u29(0x1000 | (node_id & 0x0f))),
        data_(bytes.data()),
        size_(bytes.size()){;}

    [[nodiscard]] constexpr bool has_next() const {
        return offset_ < size_;
    }



    [[nodiscard]] constexpr hal::ClassicCanFrame next(){
        if(not has_next()) __builtin_trap();

        const size_t dlc_len = std::min(size_t(size_ - offset_), size_t(8));

        auto guard = make_scope_guard([&]{
            offset_ += dlc_len;
        });

        return hal::ClassicCanFrame::from_parts(
            can_id_,
            hal::ClassicCanPayload::from_bytes(std::span(data_ + offset_, dlc_len))
        );
    }
private:
    static constexpr hal::CanExtId CAN_ID = hal::CanExtId::from_u29(0x1000);


    hal::CanExtId can_id_;
    const uint8_t * data_;
    size_t size_;
    size_t offset_;
};


}