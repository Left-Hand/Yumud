#include "pdstepper_modbus_api_facade.hpp"
#include "core/utils/bytes/buffer_cursor.hpp"

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

    static constexpr Self from_write_16(
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

    static constexpr Self from_read_16(
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



    static constexpr Self from_write_32arr(
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
        auto cursor = BufferCursor{buf};

        cursor.push_u8be(node_id);

        switch(op_code.type){
            case OpType::Write16:{
                cursor.push_u8be(0x06);
                cursor.push_u16be(static_cast<uint16_t>(command));
                cursor.push_u16be(static_cast<uint16_t>(context.u16x1));
                break;
            }

            case OpType::Read16:{
                cursor.push_u8be(0x04);
                cursor.push_u16be(static_cast<uint16_t>(command));
                cursor.push_u16be(static_cast<uint16_t>(context.u16x1));
                break;
            }

            case OpType::WriteU32Array:{
                cursor.push_u8be(0x10);
                cursor.push_u16be(static_cast<uint16_t>(command));

                const size_t len = op_code.length;
                cursor.push_u16be(static_cast<uint16_t>(len * 2));
                cursor.push_u8be(static_cast<uint8_t>(len * 4));
                for(size_t i = 0; i < len; i++){
                    cursor.push_u32be(context.u32_values[i]);
                }
                break;
            }
        }

        const size_t num_bytes = cursor.ptr - buf;
        (void)srz.take_cursor_and_inc(num_bytes);

        const uint16_t checksum = modbus::ChecksumBuilder::from_default()
            .push_bytes(std::span(buf, num_bytes))
            .finalize();

        cursor.push_u16le(static_cast<uint16_t>(checksum));

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

    static constexpr uint8_t * push_f32(uint8_t * ptr, const float value){
        static constexpr size_t LEN = sizeof(float);
        static_assert(LEN == 4);
        static_assert(std::endian::native == std::endian::little);
        const uint32_t u32_value = std::bit_cast<uint32_t>(value);
        return u8ptr_push_u32le(ptr, u32_value);
    }

};

struct [[nodiscard]] ModbusPacketFactoryBackend final{

    using Packet = ErasedModbusPacket;
    struct State{
        uint8_t node_id;
    };

    static constexpr Packet write_16(
        const State state, 
        const Command command, 
        const uint16_t data
    ){
        return Packet::from_write_16(state.node_id, command, data);
    }

    static constexpr Packet read_16(
        const State state, 
        const Command command, 
        const uint16_t quantity
    ){
        return Packet::from_read_16(state.node_id, command, quantity);
    }

    static constexpr Packet write_32arr(
        const State state, 
        const Command command, 
        std::span<const uint32_t> values
    ){
        return Packet::from_write_32arr(state.node_id, command, values);
    }
};

}