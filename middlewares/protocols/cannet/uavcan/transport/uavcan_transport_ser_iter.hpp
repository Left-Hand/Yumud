#pragma once

#include "uavcan_transport_primitive.hpp"
#include "primitive/can/bxcan_frame.hpp"
#include "core/utils/scope_guard.hpp"
#include "../uavcan_crc.hpp"


namespace ymd::uavcan{

[[nodiscard]] static constexpr 
uint8_t * u8ptr_push_be_u16(uint8_t * ptr, uint16_t val){
    ptr[0] = uint8_t(val >> 8);
    ptr[1] = uint8_t(val & 0xff);
    return ptr + 2;
}

[[nodiscard]] static constexpr 
uint8_t * u8ptr_push_u8(uint8_t * ptr, uint8_t val){
    ptr[0] = uint8_t(val & 0xff);
    return ptr + 1;
}

struct [[nodiscard]] Bytes2CanFrameSlicingIterator final{


    struct [[nodiscard]] Parameters final{
        Header header;
        std::span<const uint8_t> src_bytes;
        Signature signature;
        TransferId transfer_id;
    };

    struct [[nodiscard]] alignas(4) State final{
        uint16_t bytes_offset;
        bool toggle_bit;

        static constexpr State zero() {
            return State{0, false};
        }
    };

    explicit constexpr Bytes2CanFrameSlicingIterator(const Parameters & params):params_(params){
        if(params_.src_bytes.data() == nullptr)
            __builtin_trap();
        if(params_.src_bytes.size() > 256) // UAVCAN 有最大传输大小限制
            __builtin_trap();
    }

    [[nodiscard]] constexpr hal::ClassicCanFrame next() {
        const auto src_bytes = params_.src_bytes;
        const size_t bytes_offset = state_.bytes_offset;

        if(bytes_offset >= src_bytes.size()){
            //should not reach here, or lib it self has bug
            __builtin_trap();
        }

        const size_t pending_length = src_bytes.size() - bytes_offset;

        std::array<uint8_t, 8> payload;
        uint8_t * cursor = payload.data();

        if(bytes_offset == 0){//first frame
            if(pending_length > 7){//multi frame
                const auto checksum = CrcBuilder::from_default()
                    .push_signature(params_.signature)
                    .push_bytes(src_bytes)
                    .finalize()
                ;

                cursor = u8ptr_push_be_u16(cursor, checksum);
            }
        }

        // 1：对于Single frame transfer ，start of transfer 位永远为 1。
        // 2：对于Multiframe transfer ，如果当前帧是数据包的首帧，该位为1 ，否则为0。
        const bool is_start_of_transfer = (bytes_offset == 0);

        // 1：对于Single frame transfer ，End of transfer 这一 bit 位永远为 1。
        // 2：对于Multiframe transfer ，如果当前帧是数据包的最后一帧，该位为1 ，否则为0。
        const auto [copy_len, is_end_of_transfer] = [&]() -> std::pair<size_t, bool>{ 
            const int32_t available_capacity = 7 - (cursor - payload.data());
            
            if(static_cast<int32_t>(pending_length) <= available_capacity){
                return std::make_pair(pending_length, true);
            }else{
                return std::make_pair(size_t(available_capacity), false);
            }
        }();


        const auto guard = make_scope_guard([&](){
            state_.bytes_offset += copy_len;
            state_.toggle_bit = !state_.toggle_bit;
        });

        {
            const uint8_t * src_ptr = src_bytes.data() + bytes_offset;
            for(size_t i = 0; i < copy_len; i++){
                cursor[i] = src_ptr[i];
            }
            cursor += copy_len;
        }

        const auto tail_byte = TailByte{
            .transfer_id = params_.transfer_id.bits,
            .toggle = state_.toggle_bit,
            .is_end_of_transfer = is_end_of_transfer,
            .is_start_of_transfer = is_start_of_transfer
        };

        cursor = u8ptr_push_u8(cursor, tail_byte.to_bits());

        return hal::ClassicCanFrame::from_parts(
            params_.header.to_can_id(),
            hal::ClassicCanPayload::from_bytes(std::span(payload.data(), cursor))
        );
    }

    [[nodiscard]] constexpr bool has_next() const noexcept {
        return state_.bytes_offset < params_.src_bytes.size();
    }

private:
    const Parameters params_;
    State state_ = State::zero();
};

struct [[nodiscard]] SlicingIteratorSpawner final{
    struct [[nodiscard]] State{
        TransferId transfer_id;
    };

    State state;

    [[nodiscard]] constexpr Bytes2CanFrameSlicingIterator spawn(
        const Header header,
        const std::span<const uint8_t> src_bytes,
        const Signature signature
    ) {
        auto gaurd = make_scope_guard([&](){
            state.transfer_id = state.transfer_id.rounded_inc();
        });
        return Bytes2CanFrameSlicingIterator({
            .header = header,
            .src_bytes = src_bytes,
            .signature = signature,
            .transfer_id = state.transfer_id
        });
    }
};

}