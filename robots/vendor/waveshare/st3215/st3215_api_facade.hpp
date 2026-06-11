#pragma once

#include "st3215_primitive.hpp"
#include "st3215_utils.hpp"
#include "core/utils/bytes/buffer_cursor.hpp"

namespace ymd::robots::waveshare::st3215{


namespace details{

template<typename T>
struct is_boardcast_msg{static constexpr bool value = false;};

template<>
struct is_boardcast_msg<ins_msgs::WriteData>{static constexpr bool value = true;};

template<>
struct is_boardcast_msg<ins_msgs::Action>{static constexpr bool value = true;};

template<>
struct is_boardcast_msg<ins_msgs::SyncWrite>{static constexpr bool value = true;};

template<typename T>
static constexpr bool is_boardcast_msg_v = is_boardcast_msg<T>::value;

}

template<typename Msg>
static constexpr ServoId remap_id(const ServoId id){
    constexpr bool is_boardcast = details::is_boardcast_msg_v<Msg>;
    if(is_boardcast){
        return BOARDCAST_SERVOID;
    }else{
        return id;
    }
}


struct ErasedPacket{
    using Self = ErasedPacket;


    ServoId id;
    uint8_t payload_len;
    Instruction instruction;
    union Conetxt{
        ins_msgs::Ping ping;
        ins_msgs::ReadData read_data;
        ins_msgs::WriteData write_data;
    }context;


    static constexpr Conetxt make_context(const ins_msgs::Ping & msg){
        return Conetxt{.ping = msg};
    };

    static constexpr Conetxt make_context(const ins_msgs::ReadData & msg){
        return Conetxt{.read_data = msg};
    };

    static constexpr Conetxt make_context(const ins_msgs::WriteData & msg){
        return Conetxt{.write_data = msg};
    };


    template<typename Msg>
    static constexpr Self from(
        const ServoId id,
        Msg && msg
    ){
        Self self{
            .id = remap_id<std::decay_t<Msg>>(id),
            .payload_len = static_cast<uint8_t>(msg.payload_length() + 2),
            .instruction = Instruction{std::decay_t<Msg>::INSTRUCTION},
            .context = make_context(msg)
        };

        return self;
    }

    template<typename Serializer>
    constexpr Result<void, typename Serializer::Error> 
    serialize(Serializer & srz) const {
        auto & self = *this;

        const size_t buf_len = self.payload_len + 4;
        if(const auto res = srz.compatible_with_length(buf_len);
            res.is_err()) return Err(res.unwrap_err());
            
        uint8_t * buf = srz.take_cursor_and_inc(buf_len);
        auto cursor = BufferCursor{buf};

        cursor.push_u16be(0xffff);
        cursor.push_u8be(self.id.bits);
        cursor.push_u8be(self.payload_len);
        cursor.push_u8be(self.instruction.to_u8());

        switch(self.instruction.kind()){
            case Instruction::Ping:
                //nothing to write
                break;
            case Instruction::ReadData:{
                const auto & msg = self.context.read_data;
                cursor.push_u8be(msg.base_addr);
                cursor.push_u8be(msg.len);
                break;
            }
            case Instruction::WriteData:{
                const auto & msg = self.context.write_data;
                cursor.push_u8be(msg.base_addr);
                cursor.push_bytes(msg.data);
                break;
            }
            case Instruction::RegWrite:
                break;
            case Instruction::Action:
                //nothing to write
                break;
            case Instruction::Reset:
                break;
            case Instruction::SyncRead:
                break;
            case Instruction::SyncWrite:
                break;
            
        }

        const uint8_t checksum = ChecksumBuilder::from_default()
            .push_bytes(std::span(buf + 2, self.payload_len + 1))
            .finalize();

        cursor.push_u8be(checksum);

        return Ok();
    }
};


struct FrameFactoryBackend{
    struct State{
        ServoId id;
    };

    template<typename Msg>
    static constexpr ErasedPacket convert(const State & state, Msg && msg){
        return ErasedPacket::from(state.id, msg);
    }


};


template<typename Backend>
struct [[nodiscard]] ClientApiFacade final{

    using State = typename Backend::State;
    State state;

    [[nodiscard]] constexpr auto ping(this auto && self) noexcept{
        return Backend::convert(self.state, ins_msgs::Ping{});
    }

    [[nodiscard]] constexpr auto read_data(this auto && self,
        ins_msgs::ReadData && msg
    ) noexcept{
        return Backend::convert(self.state, msg);
    }

    [[nodiscard]] constexpr auto write_data(this auto && self,
        ins_msgs::WriteData && msg
    ) noexcept{
        return Backend::convert(self.state, msg);
    }


};


using FrameFactory = ClientApiFacade<FrameFactoryBackend>;

}