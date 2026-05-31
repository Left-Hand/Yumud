#pragma once


#include "cds55xx_primitive.hpp"
#include "cds55xx_utils.hpp"



namespace ymd::robots::dfrobot::cds55xx{

struct [[nodiscard]] Regset{
    using RegAddr = uint8_t;

};


struct [[nodiscard]] FrameFactory final{
    const ServoId id;

    template<size_t N>
    [[nodiscard]] constexpr auto write_data(
        const uint8_t addr, 
        const std::span<const uint8_t, N> data
    ){
        return assembly_frame(id, [&]{return make_write_data_payload(addr, std::span(data));});
    }

    template<size_t N>
    [[nodiscard]] constexpr auto async_write_data(
        const uint8_t addr, 
        const std::span<const uint8_t, N> data
    ){
        return assembly_frame(id, [&]{return make_async_write_data_payload(addr, std::span(data));});
    }


    template<typename Dummy = void>
    [[nodiscard]] constexpr auto read_data(
        const uint8_t addr, 
        const uint8_t len
    ){
        return assembly_frame(id, [&]{return make_read_data_payload(addr, len);});
    }

    template<typename Dummy = void>
    [[nodiscard]] constexpr auto invoke_async(
    ){
        return assembly_frame(id, [&]{return make_invoke_async_payload();});
    }

    template<typename Dummy = void>
    [[nodiscard]] constexpr auto ping(
    ){
        return assembly_frame(id, [&]{return make_ping_payload();});
    }

    template<typename Dummy = void>
    [[nodiscard]] constexpr auto reset(
    ){
        return assembly_frame(id, [&]{return make_reset_payload();});
    }




    template<size_t N>
    static constexpr std::array<uint8_t, N + 5> assembly_payload(const ServoId id, std::span<const uint8_t, N> payload){
        std::array<uint8_t, N + 5> buf;
        auto filler = BytesFiller(buf);
        filler.push_bytes(std::span(HEADER_TOKEN));
        filler.push_byte(id.count);
        const auto len = payload.size() + 1;
        filler.push_byte(len);
        filler.push_bytes(payload);
        filler.push_byte(calc_checksum(id.count, payload));
        return buf;
    }

    template<typename Fn>
    static constexpr auto assembly_frame(const ServoId id, Fn && fn){
        const auto payload = std::forward<Fn>(fn)();
        const auto frame = assembly_payload(id, std::span(payload));
        return frame;
    }

    template<size_t N>
    static constexpr std::array<uint8_t, N + 2> make_write_data_payload(const uint8_t addr, std::span<const uint8_t, N> data){
        std::array<uint8_t, N + 2> buf;
        auto filler = BytesFiller(buf);
        filler.push_byte(Instruction(Instruction::Kind::WriteData).to_u8());
        filler.push_byte(addr);
        filler.push_bytes(data);
        return buf;
    }

    template<size_t N>
    static constexpr std::array<uint8_t, N + 2> make_sync_write_payload(const uint8_t addr, std::span<const uint8_t, N> data){
        std::array<uint8_t, N + 2> buf;
        auto filler = BytesFiller(buf);
        filler.push_byte(Instruction(Instruction::Kind::WriteData).to_u8());
        filler.push_byte(addr);
        filler.push_bytes(data);
        return buf;
    }

    static constexpr std::array<uint8_t, 3> make_read_data_payload(const uint8_t addr, const uint8_t len){
        std::array<uint8_t, 3> buf;
        auto filler = BytesFiller(buf);
        filler.push_byte(Instruction(Instruction::Kind::ReadData).to_u8());
        filler.push_byte(addr);
        filler.push_byte(len);
        return buf;
    }

    template<size_t N>
    static constexpr std::array<uint8_t, N + 2> make_async_write_data_payload(const uint8_t addr, std::span<const uint8_t, N> data){
        std::array<uint8_t, N + 2> buf;
        auto filler = BytesFiller(buf);
        filler.push_byte(Instruction(Instruction::Kind::AsyncWrite).to_u8());
        filler.push_byte(addr);
        filler.push_bytes(data);
        return buf;
    }

    static constexpr std::array<uint8_t, 1> make_invoke_async_payload(){
        std::array<uint8_t, 1> buf;
        buf[0] = (Instruction(Instruction::Kind::InvokeAsync).to_u8());
        return buf;
    }

    static constexpr std::array<uint8_t, 1> make_ping_payload(){
        std::array<uint8_t, 1> buf;
        buf[0] = (Instruction(Instruction::Kind::Ping).to_u8());
        return buf;
    }

    static constexpr std::array<uint8_t, 1> make_reset_payload(){
        std::array<uint8_t, 1> buf;
        buf[0] = (Instruction(Instruction::Kind::Reset).to_u8());
        return buf;
    }
};


struct [[nodiscard]] Transport final{

    template<typename T>
    struct [[nodiscard]] OperWrapper{
        T value;
    };

    struct [[nodiscard]] Oper{
        template<size_t N>
        [[nodiscard]] constexpr auto write_data(const uint8_t addr, const std::array<uint8_t, N>data) const noexcept {
            return make_oper(FrameFactory{id_}.write_data(addr, std::span(data)));
        }
    private:
        ServoId id_;
    };

    const auto operate() const noexcept {
        return oper_;
    }
private:
    Oper oper_;
    template<typename T>
    OperWrapper<T> make_oper(const T && value){
        return {value};
    }
};

}