#include "../pdstepper_primitive.hpp"
#include "middlewares/protocols/modbus/modbus_msgs.hpp"
#include "middlewares/protocols/modbus/modbus_serialize.hpp"


namespace ymd::robots::pdstepper{


struct [[nodiscard]] ErasedPacket final{
    using Self = ErasedPacket;

    enum class OpType:uint8_t{
        Write16,
        Read16
    };

    uint8_t node_id;

    struct OpCode{
        uint8_t length;
        OpType type;
    } op_code;

    Command command;
    union{
        uint16_t u16x1;
    }context;

    static constexpr Self from_write16(
        const uint8_t node_id,
        const Command command,
        const uint16_t data
    ){
        return Self{
            .node_id = node_id,
            .op_code = {
                .length = 0,
                .type = OpType::Write16
            },
            .command = command,
            .context ={.u16x1 = data}
        };
    }

    static constexpr Self from_read16(
        const uint8_t node_id,
        const Command command,
        const uint16_t quantity
    ){
        return Self{
            .node_id = node_id,
            .op_code = {
                .length = 0,
                .type = OpType::Read16
            },
            .command = command,
            .context ={.u16x1 = quantity}
        };
    }

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize(Serializer & srz) const {
        switch(op_code.type){
            case OpType::Write16:{
                const auto msg = modbus::req_msgs::WriteSingleHoldingRegister{
                    .reg_addr = static_cast<uint8_t>(command),
                    .reg_value = context.u16x1
                };

                return modbus::serialize_rtu_msg(srz, msg, node_id);
                break;
            }

            case OpType::Read16:{
                const auto msg = modbus::req_msgs::ReadInputRegisters{
                    .base_addr = static_cast<uint8_t>(command),
                    .quantity = context.u16x1
                };

                return modbus::serialize_rtu_msg(srz, msg, node_id);
                break;
            }
        }

        __builtin_unreachable();
    }
};

struct ModbusPacketBackend{
    struct State{
        uint8_t node_id;
    };

    static constexpr ErasedPacket write16(
        const State state, 
        const Command command, 
        const uint16_t data
    ){
        return ErasedPacket::from_write16(state.node_id, command, data);
    }

    static constexpr ErasedPacket read16(
        const State state, 
        const Command command, 
        const uint16_t data
    ){
        return ErasedPacket::from_read16(state.node_id, command, data);
    }
};


template<typename Backend>
struct ClientApiFacade{
    using State = typename Backend::State;

    State state;

    constexpr auto calibrate_encoder() const {
        return Backend::write16(state, Command::CalibrateEncoder, 0x01);
    }

    constexpr auto reset() const {
        return Backend::write16(state, Command::Reset, 0x01);
    }

    constexpr auto restore_factory() const {
        return Backend::write16(state, Command::RestoreFactory, 0x01);
    }

    constexpr auto get_swhw_version() const {
        return Backend::read16(state, Command::GetSoftHardVer, 0x01);
    }

    constexpr auto get_flux() const {
        return Backend::read16(state, Command::GetFlux, 0x02);
    }

    constexpr auto get_phase_resind() const {
        return Backend::read16(state, Command::GetPhaseResInd, 0x04);
    }

    constexpr auto get_phase_current() const {
        return Backend::read16(state, Command::GetPhaseCurrent, 0x04);
    }


};


}