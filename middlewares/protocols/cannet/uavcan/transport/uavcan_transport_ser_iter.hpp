#pragma once

#include "uavcan_transport_primitive.hpp"
#include "primitive/can/bxcan_frame.hpp"
#include "core/utils/scope_guard.hpp"
#include "../uavcan_crc.hpp"


namespace ymd::uavcan{



struct [[nodiscard]] Bytes2CanFrameSlicingIterator final{


    struct Parameters{
        Header header;
        std::span<const uint8_t> bytes;
        Signature signature;
        TransferId transfer_id;
    };

    struct [[nodiscard]] State final{
        size_t bytes_offset;
        bool toggle_bit;

        static constexpr State zero() {
            return State{0, false};
        }
    };

    explicit constexpr Bytes2CanFrameSlicingIterator(const Parameters & paras):paras_(paras){
        if(paras_.bytes.data() == nullptr)
            __builtin_trap();
        if(paras_.bytes.size() > 256) // UAVCAN 有最大传输大小限制
            __builtin_trap();
    }

    [[nodiscard]] constexpr hal::BxCanFrame next() {
        const auto bytes = paras_.bytes;
        const auto bytes_offset = state_.bytes_offset;

        if(bytes_offset >= bytes.size()){
            //should not reach here, or lib it self has bug
            __builtin_trap();
        }

        const size_t pending_length = bytes.size() - bytes_offset;

        std::array<uint8_t, 8> payload;
        size_t len = 0;

        if(bytes_offset == 0){//first frame
            if(pending_length > 7){//multi frame
                const auto crc = CrcBuilder(0xffff)
                    .push_signature(paras_.signature)
                    .push_bytes(bytes)
                    .get()
                ;
                payload[len++] = static_cast<uint8_t>((crc >> 8) & 0xff);
                payload[len++] = static_cast<uint8_t>(crc & 0xff);
            }
        }

        // 1：对于Single frame transfer ，start of transfer 位永远为 1。
        // 2：对于Multiframe transfer ，如果当前帧是数据包的首帧，该位为1 ，否则为0。
        const bool is_start_of_transfer = (bytes_offset == 0);

        // 1：对于Single frame transfer ，End of transfer 这一 bit 位永远为 1。
        // 2：对于Multiframe transfer ，如果当前帧是数据包的最后一帧，该位为1 ，否则为0。
        const auto [copy_len, is_end_of_transfer] = [&]() -> std::tuple<size_t, bool>{ 
            if(pending_length <= 7 - len){
                return std::make_tuple(pending_length, true);
            }else{
                return std::make_tuple(7 - len, false);
            }
        }();
        const auto guard = make_scope_guard([&](){
            state_.bytes_offset += copy_len;
            state_.toggle_bit = !state_.toggle_bit;
        });

        const auto bytes_need_copy = std::span(bytes.begin() + bytes_offset, copy_len);

        std::copy(bytes_need_copy.begin(), bytes_need_copy.end(), payload.begin() + len);
        len += copy_len;

        const auto tail_byte = TailByte{
            .transfer_id = paras_.transfer_id.bits,
            .toggle = state_.toggle_bit,
            .is_end_of_transfer = is_end_of_transfer,
            .is_start_of_transfer = is_start_of_transfer
        };

        payload[len] = tail_byte.to_bits();
        len += 1;

        return hal::BxCanFrame::from_parts(
            paras_.header.to_can_id(),
            hal::BxCanPayload::from_bytes(std::span(payload.data(), len))
        );
    }

    [[nodiscard]] constexpr bool has_next() const {
        return state_.bytes_offset < paras_.bytes.size();
    }

private:
    Parameters paras_;
    State state_ = State::zero();
};

struct [[nodiscard]] SlicingIteratorSpawner final{
    struct [[nodiscard]] State{
        TransferId transfer_id;
    };

    State state;

    [[nodiscard]] constexpr Bytes2CanFrameSlicingIterator spawn(
        const Header header,
        const std::span<const uint8_t> bytes,
        const Signature signature
    ) {
        auto gaurd = make_scope_guard([&](){
            state.transfer_id = state.transfer_id.rounded_inc();
        });
        return Bytes2CanFrameSlicingIterator({
            .header = header,
            .bytes = bytes,
            .signature = signature,
            .transfer_id = state.transfer_id
        });
    }
};

}