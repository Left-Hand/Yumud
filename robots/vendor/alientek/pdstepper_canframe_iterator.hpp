#pragma once

#include "primitive/can/can_frame.hpp"
#include "core/utils/scope_guard.hpp"

namespace ymd::robots::pdstepper{

struct [[nodiscard]] CanframeSlicingIterator final{
    constexpr CanframeSlicingIterator(std::span<const uint8_t> bytes):
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
            CAN_ID,
            hal::ClassicCanPayload::from_bytes(std::span(data_ + offset_, dlc_len))
        );
    }
private:
    static constexpr hal::CanExtId CAN_ID = hal::CanExtId::from_u29(0x1000);

    const uint8_t * data_;
    size_t size_;
    size_t offset_;
};


}