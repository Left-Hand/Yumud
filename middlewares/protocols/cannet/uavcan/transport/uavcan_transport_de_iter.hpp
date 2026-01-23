#pragma once

#include "uavcan_transport_primitive.hpp"
#include "primitive/can/bxcan_frame.hpp"
#include "core/utils/scope_guard.hpp"
#include "../uavcan_crc.hpp"

#if 0
namespace ymd::uavcan{


enum class AssemblyError {
    EmptyFrame,
    TransferIdMismatch,
    ToggleBitSequenceError,
    OutOfOrderFrame,
    BufferOverflow,
};

struct [[nodiscard]] CanFrameAssemblyIterator final {
    enum class FsmState:uint8_t{
        Idle = 0,
        R
    };
    struct FsmStatus{
        using Self = FsmStatus;
        uint8_t __resv__:5;
        uint8_t toggle:1;
        uint8_t is_end_of_transfer:1;
        uint8_t is_start_of_transfer:1;

        static constexpr Self zero(){
            return std::bit_cast<Self>(uint8_t(0));
        }
    };


    struct [[nodiscard]] State final {
        uint16_t bytes_received;
        uint16_t crc_accumulator; // 用于多帧传输的CRC计算
        FsmStatus fsm_status;
        static constexpr State from_default() {
            return State{
                .bytes_received = 0,
                .crc_accumulator = 0xFFFF,
                .fsm_status = FsmStatus::zero()
            };
        }
    };
    
    struct [[nodiscard]] Paraments final {
        std::span<uint8_t> buffer; // 输出缓冲区
    };
    
    explicit constexpr CanFrameAssemblyIterator(Paraments paras)
        : paras_(paras)
    {
        if (paras_.buffer.data() == nullptr) {
            __builtin_trap();
        }
    }
    
    [[nodiscard]] Result<bool, AssemblyError> poll(const hal::BxCanFrame& frame) {
        
        // 解析CAN帧
        const auto payload_bytes = frame.payload().bytes();
        if (payload_bytes.empty()) {
            return Err(AssemblyError::EmptyFrame);
        }
        
        // 提取tail byte
        const auto tail_byte = TailByte::from_bits(payload_bytes.back());
        
        // 验证toggle位
        if (!validate_toggle_bit(tail_byte.toggle)) {
            return Err(AssemblyError::ToggleBitSequenceError);
        }
        
        // 处理数据部分
        const size_t data_length = payload_bytes.size() - 1; // 减去tail byte
        if (data_length == 0 && !tail_byte.is_start_of_transfer && !tail_byte.is_end_of_transfer) {
            return Err(AssemblyError::EmptyFrame);
        }
        
        // 检查缓冲区空间
        if (state_.bytes_received + data_length > paras_.buffer.size()) {
            return Err(AssemblyError::BufferOverflow);
        }
        
        // 处理首帧
        if (tail_byte.is_start_of_transfer) {
            state_ = State::from_default();

            // 如果是多帧传输，前两个字节是CRC
            if (data_length > 7) {
                // 多帧传输，前两个字节是CRC
                if (data_length < 2) {
                    return Err(AssemblyError::EmptyFrame);
                }
                expected_crc_ = (static_cast<uint16_t>(payload_bytes[0]) << 8) | 
                                static_cast<uint16_t>(payload_bytes[1]);
                data_offset_ = 2; // 跳过CRC字节
            } else {
                data_offset_ = 0;
                expected_crc_ = 0;
            }
        } else {
            data_offset_ = 0;
        }
        
        // 复制数据到缓冲区
        const size_t copy_start = data_offset_;
        const size_t copy_length = data_length - data_offset_;
        
        if (copy_length > 0) {
            std::copy_n(
                payload_bytes.begin() + copy_start,
                copy_length,
                paras_.buffer.begin() + state_.bytes_received
            );
            
            // 更新CRC（如果是多帧传输）
            if (expected_crc_ != 0) {
                accumulate_crc(std::span<const uint8_t>(
                    payload_bytes.begin() + copy_start,
                    copy_length
                ));
            }
            
            state_.bytes_received += copy_length;
        }
        
        // 更新状态
        state_.expected_toggle = !state_.expected_toggle;
        
        // 检查是否完成
        if (tail_byte.is_end_of_transfer) {
            return Ok(true);
        }
        
        return Ok(false);
    }
    
    [[nodiscard]] constexpr size_t bytes_received() const {
        return state_.bytes_received;
    }
    
    
    [[nodiscard]] constexpr std::span<const uint8_t> assembled_data() const {
        return std::span<const uint8_t>(
            paras_.buffer.data(),
            state_.bytes_received
        );
    }
    
    constexpr void reset() {
        state_ = State::from_default();
    }
    
private:
    
    void accumulate_crc(std::span<const uint8_t> bytes) {
        state_.crc_accumulator = CrcBuilder(state_.crc_accumulator).push_bytes(bytes);
    }

    
    Paraments paras_;
    State state_ = State::from_default();
};

struct [[nodiscard]] AssemblyIteratorSpawner final {
    struct [[nodiscard]] State {
        // 可以保存一些状态信息，如最近处理的transfer id等
    };
    
    State state;
    
    [[nodiscard]] CanFrameAssemblyIterator spawn(
        std::span<uint8_t> buffer,
        uint32_t data_type_signature = 0
    ) {
        return CanFrameAssemblyIterator({
            .buffer = buffer,
            .data_type_signature = data_type_signature
        });
    }
};
}

#endif