#include "../pdstepper_primitive.hpp"
#include "middlewares/protocols/modbus/modbus_msgs.hpp"
#include "middlewares/protocols/modbus/modbus_serialize.hpp"
#include "core/math/float/fp32.hpp"

namespace ymd::robots::pdstepper{


struct [[nodiscard]] ErasedModbusPacket final{
    using Self = ErasedModbusPacket;

    enum class OpType:uint8_t{
        Write16,
        Read16,
        // WriteFLoat,
        WriteU32Array,
    };

    static constexpr size_t MAX_U32ARRAY_LEN = 3;
    static constexpr size_t MAX_MODBUS_PACKET_LEN = 32;

    using u32array  = std::array<float, MAX_U32ARRAY_LEN>;

    uint8_t node_id;


    struct OpCode{
        uint8_t length;
        OpType type;
    } op_code;

    Command command;
    union{
        uint16_t u16x1;
        u32array u32_values;
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



    static constexpr Self from_write32arr(
        const uint8_t node_id,
        const Command command,
        std::span<const uint32_t> values
    ){
        return Self{
            .node_id = node_id,
            .op_code = {
                .length = static_cast<uint8_t>(values.size()),
                .type = OpType::WriteU32Array
            },
            .command = command,
            .context ={.u32_values = clone_u32arr(values)}
        };
    }

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize(Serializer & srz) const {
        if(const auto res = srz.compatible_with_length(MAX_MODBUS_PACKET_LEN);
            res.is_err()) return Err(res.unwrap_err());
        uint8_t * buf = srz.take_cursor_and_inc(0);
        uint8_t * cursor = buf;

        cursor = ptr_push_u8(cursor, node_id);

        switch(op_code.type){
            case OpType::Write16:{
                cursor = ptr_push_u8(cursor, 0x06);
                cursor = ptr_push_u16be(cursor, static_cast<uint16_t>(command));
                cursor = ptr_push_u16be(cursor, static_cast<uint16_t>(context.u16x1));
                break;
            }

            case OpType::Read16:{
                cursor = ptr_push_u8(cursor, 0x04);
                cursor = ptr_push_u16be(cursor, static_cast<uint16_t>(command));
                cursor = ptr_push_u16be(cursor, static_cast<uint16_t>(context.u16x1));
                break;
            }

            case OpType::WriteU32Array:{
                cursor = ptr_push_u8(cursor, 0x10);
                cursor = ptr_push_u16be(cursor, static_cast<uint16_t>(command));

                const size_t len = op_code.length;
                cursor = ptr_push_u16be(cursor, static_cast<uint16_t>(len * 2));
                cursor = ptr_push_u8(cursor, static_cast<uint8_t>(len * 4));
                for(size_t i = 0; i < len; i++){
                    cursor = ptr_push_u32be(cursor, context.u32_values[i]);
                }
                break;
            }
        }

        const size_t num_bytes = cursor - buf;
        (void)srz.take_cursor_and_inc(num_bytes);

        const uint16_t checksum = modbus::ChecksumBuilder::from_default()
            .push_bytes(std::span(buf, num_bytes))
            .finalize();

        cursor = ptr_push_u16le(cursor, static_cast<uint16_t>(checksum));

        (void)srz.take_cursor_and_inc(2);

        return Ok();
    }

private:
    template<typename T>
    static constexpr u32array clone_u32arr(
        std::span<const T> values
    ){
        static_assert(sizeof(T) == 4);
        u32array ret;
        const size_t len = std::min(values.size(), MAX_U32ARRAY_LEN);
        for(size_t i = 0; i < len; i++){
            ret[i] = std::bit_cast<uint32_t>(values[i]);
        }
        return ret;
    }

    static constexpr uint8_t * ptr_push_f32(uint8_t * ptr, const float value){
        static constexpr size_t LEN = sizeof(float);
        static_assert(LEN == 4);
        static_assert(std::endian::native == std::endian::little);
        const auto u8x4 = std::bit_cast<std::array<uint8_t, 4>>(value);
        for(size_t i = 0; i < LEN; i++) ptr[i] = u8x4[i];
        return ptr + LEN;
    }

    static constexpr uint8_t * ptr_push_u8(uint8_t * ptr, const uint8_t value){
        ptr[0] = value;
        return ptr + 1;
    }

    static constexpr uint8_t * ptr_push_u16be(uint8_t * ptr, const uint16_t value){
        ptr[0] = uint8_t(value >> 8);
        ptr[1] = uint8_t(value);
        return ptr + 2;
    }

    static constexpr uint8_t * ptr_push_u32be(uint8_t * ptr, const uint32_t value){
        ptr[0] = uint8_t(value >> 24);
        ptr[1] = uint8_t(value >> 16);
        ptr[2] = uint8_t(value >> 8);
        ptr[3] = uint8_t(value);
        return ptr + 4;
    }

    static constexpr uint8_t * ptr_push_u16le(uint8_t * ptr, const uint16_t value){
        ptr[0] = uint8_t(value);
        ptr[1] = uint8_t(value >> 8);
        return ptr + 2;
    }

};

struct ModbusPacketFactoryBackend{

    using Packet = ErasedModbusPacket;
    struct State{
        uint8_t node_id;
    };

    static constexpr Packet write16(
        const State state, 
        const Command command, 
        const uint16_t data
    ){
        return Packet::from_write16(state.node_id, command, data);
    }

    static constexpr Packet read16(
        const State state, 
        const Command command, 
        const uint16_t quantity
    ){
        return Packet::from_read16(state.node_id, command, quantity);
    }

    static constexpr Packet write32arr(
        const State state, 
        const Command command, 
        std::span<const uint32_t> values
    ){
        return Packet::from_write32arr(state.node_id, command, values);
    }
};


template<typename Backend>
struct ClientApiFacade{
    using State = typename Backend::State;

    State state;

    constexpr auto calibrate_encoder(this auto && self) {
        return Backend::write16(self.state, Command::CalibrateEncoder, 0x01);
    }

    constexpr auto reset(this auto && self) {
        return Backend::write16(self.state, Command::Reset, 0x01);
    }

    constexpr auto restore_factory(this auto && self) {
        return Backend::write16(self.state, Command::RestoreFactory, 0x01);
    }

    constexpr auto get_swhw_version(this auto && self) {
        return Backend::read16(self.state, Command::GetSoftHardVer, 0x01);
    }

    constexpr auto get_flux(this auto && self) {
        return Backend::read16(self.state, Command::GetFlux, 0x02);
    }


    // 5.3.3 读取相电阻和相电感指令
    constexpr auto get_phase_resind(this auto && self) {
        return Backend::read16(self.state, Command::GetPhaseResInd, 0x04);
    }


    // 5.3.4 读取相电流指令
    constexpr auto get_phase_current(this auto && self) {
        return Backend::read16(self.state, Command::GetPhaseCurrent, 0x04);
    }

    // 5.3.5 读取总线电压指令
    constexpr auto get_busbar_voltage(this auto && self) {
        return Backend::read16(self.state, Command::GetBusbarVoltage, 0x04);
    }

    // 5.3.22 设置位置环 PID 参数指令
    constexpr auto set_position_pid_paraments(
        this auto && self, 
        std::array<uint32_t, 3> paraments
    ) {
        return Backend::write32arr(self.state, Command::SetPositionPidParaments, std::span(paraments));
    }

};


}