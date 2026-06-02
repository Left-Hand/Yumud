#pragma once

#include "briter_modbus_primitive.hpp"
#include "middlewares/protocols/modbus/modbus_msgs.hpp"

namespace ymd::robots::briter{


template<typename Backend>
struct [[nodiscard]] ClientApiFacade final{

    using State = typename Backend::State;
    State state;

    constexpr auto heartbeat(uint16_t rolling_counter_value) const {
        return state.write_16(
            RegAddr::Heartbeat, 
            rolling_counter_value
        );
    } 

    constexpr auto set_current(const CurrentCode code) const {
        return state.write_16(
            RegAddr::SetCurrent, 
            code.bits
        );
    } 

    constexpr auto set_erpm(const ErpmCode code) const {
        return state.write_32(
            RegAddr::SetSpeedLow,
            code.bits
        );
    } 

private:
};

enum class OpCode:uint8_t{
    Write16,
    Write32
};

struct ErasedModbusMsg{
    using Self = ErasedModbusMsg;

    uint8_t node_id;
    OpCode op_code;
    uint16_t addr;
    std::array<uint16_t, 2> args;


    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize(Serializer & serializer) const {
        using FC = modbus::FunctionCode;
        switch(op_code){
            case OpCode::Write16:{
                const auto msg = modbus::req_msgs::WriteSingleHoldingRegister{
                    .reg_addr = addr,
                    .reg_value = static_cast<uint16_t>(args[0])
                };
                return modbus::serialize_rtu_msg(serializer, msg, node_id);
            }
            case OpCode::Write32:{
                const auto msg = modbus::req_msgs::WriteMultipleRegisters{
                    .base_addr = addr,
                    .reg_values = std::span(args)
                };
                return modbus::serialize_rtu_msg(serializer, msg, node_id);
            }
        }

        __builtin_unreachable();
    }
};

struct [[nodiscard]] FrameFactoryBackend{
    struct [[nodiscard]] State{
        const uint8_t node_id;

        constexpr ErasedModbusMsg write_16(RegAddr reg_addr, const uint16_t reg_val) const {
            return ErasedModbusMsg{
                .node_id = node_id,
                .op_code = OpCode::Write16,
                .addr = static_cast<uint16_t>(reg_addr),
                .args = {reg_val, 0}
            };
        }

        constexpr ErasedModbusMsg write_32(RegAddr reg_addr, const uint32_t reg_val) const {
            return ErasedModbusMsg{
                .node_id = node_id,
                .op_code = OpCode::Write32,
                .addr = static_cast<uint16_t>(reg_addr),
                .args = {
                    static_cast<uint16_t>(reg_val >> 16),
                    static_cast<uint16_t>(reg_val),
                }
            };
        }


    };


};

using FrameFactory = ClientApiFacade<FrameFactoryBackend>;

}