#pragma once

#include <array>
#include <cstdint>
#include "primitive/can/can_frame.hpp"
#include "core/utils/bytes/buffer_cursor.hpp"
#include "middlewares/protocols/modbus/modbus_msgs.hpp"


namespace ymd::robots::briter{

struct [[nodiscard]] ErasedFrame{
    using Self = ErasedFrame;


    enum class [[nodiscard]] OpCode:uint8_t{
        Write0,
        Write16,
        Write32,
        Read16,
        Read32
    };


    uint8_t node_id;
    OpCode op_code;
    uint16_t item_id;
    uint32_t arg;

    static constexpr ErasedFrame 
    from_write_0(const uint8_t node_id, uint16_t item_id) {
        return ErasedFrame{
            .node_id = node_id,
            .op_code = OpCode::Write0,
            .item_id = static_cast<uint16_t>(item_id),
            .arg = 0
        };
    }

    static constexpr ErasedFrame 
    from_write_16(const uint8_t node_id, uint16_t item_id, const uint16_t value) {
        return ErasedFrame{
            .node_id = node_id,
            .op_code = OpCode::Write16,
            .item_id = static_cast<uint16_t>(item_id),
            .arg = value
        };
    }

    static constexpr ErasedFrame 
    from_read_16(const uint8_t node_id, uint16_t item_id) {
        return ErasedFrame{
            .node_id = node_id,
            .op_code = OpCode::Read16,
            .item_id = static_cast<uint16_t>(item_id),
            .arg = 0
        };
    }


    static constexpr ErasedFrame 
    from_write_32(const uint8_t node_id, uint16_t item_id, const uint32_t value) {
        return ErasedFrame{
            .node_id = node_id,
            .op_code = OpCode::Write32,
            .item_id = static_cast<uint16_t>(item_id),
            .arg = value
        };
    }

    static constexpr ErasedFrame 
    from_read_32(const uint8_t node_id, uint16_t item_id) {
        return ErasedFrame{
            .node_id = node_id,
            .op_code = OpCode::Read32,
            .item_id = static_cast<uint16_t>(item_id),
            .arg = 0
        };
    }



    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize(Serializer & srz) const {
        using FC = modbus::FunctionCode;
        switch(op_code){
            case OpCode::Write0:
                //should not reachable here

                #ifndef _NDEBUG
                __builtin_trap();
                #else
                //guard for last safety
                [[fallthrough]];
                #endif
            case OpCode::Write16:{
                const auto msg = modbus::req_msgs::WriteSingleHoldingRegister{
                    .reg_address = item_id,
                    .reg_value = static_cast<uint16_t>(arg)
                };
                return modbus::serialize_rtu_msg(srz, msg, node_id);
            }
            case OpCode::Write32:{
                std::array<uint16_t, 2> buf{
                    uint16_t(arg >> 16),
                    uint16_t(arg)
                };

                const auto msg = modbus::req_msgs::WriteMultipleRegisters{
                    .base_address = item_id,
                    .reg_values = std::span(buf)
                };
                return modbus::serialize_rtu_msg(srz, msg, node_id);
            }
            case OpCode::Read16:{
                const auto msg = modbus::req_msgs::ReadInputRegisters{
                    .base_address = item_id,
                    .quantity = 1
                };
                return modbus::serialize_rtu_msg(srz, msg, node_id);
            }
            case OpCode::Read32:{
                const auto msg = modbus::req_msgs::ReadInputRegisters{
                    .base_address = item_id,
                    .quantity = 2
                };
                return modbus::serialize_rtu_msg(srz, msg, node_id);
            }
        }

        __builtin_unreachable();
    }

    constexpr hal::ClassicCanFrame to_can_frame() const {
        std::array<uint8_t, 8> u8x8;
        auto cursor = BufferCursor{u8x8.data()};

        switch(op_code){
            case OpCode::Write0:{
                cursor.push_u8be(static_cast<uint8_t>(item_id));
                break;
            }
            case OpCode::Write16:{
                cursor.push_u8be(static_cast<uint8_t>(item_id));
                cursor.push_u16be(arg);
                break;
            }
            case OpCode::Write32:{
                cursor.push_u8be(static_cast<uint8_t>(item_id));
                cursor.push_u32be(arg);
                break;
            }
            case OpCode::Read16:{
                cursor.push_u8be(0x0f);
                cursor.push_u8be(static_cast<uint8_t>(item_id));
                break;
            }
            case OpCode::Read32:{
                cursor.push_u8be(0x0f);
                cursor.push_u8be(static_cast<uint8_t>(item_id));
                break;
            }
        }

        const size_t len = cursor.ptr - u8x8.data();
        return hal::ClassicCanFrame::from_parts(
            //TODO check canid type
            hal::CanStdId::from_u11(node_id),
            hal::ClassicCanPayload::from_u8x8(u8x8, hal::ClassicCanDlc::from_length(len))
        );
    }
};


}