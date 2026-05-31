#pragma once

#include <array>
#include <algorithm>
#include <bits/stl_numeric.h>

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"
#include <span>

namespace ymd::robots::dfrobot::cds55xx{



struct [[nodiscard]] Command final{
public:
    enum struct [[nodiscard]] Kind:uint8_t{
        Ping = 0x01, 
        ReadData = 0x02,
        WriteData = 0x03,
        AsyncWrite = 0x04,
        InvokeAsync = 0x05,
        Reset = 0x06,
        SyncWrite = 0x83
    };

    using enum Kind;

    constexpr Command(Kind kind):kind_(kind){}

    constexpr uint8_t to_u8() const noexcept {return static_cast<uint8_t>(kind_);}
private:
    Kind kind_;
};

static constexpr auto HEADER_TOKEN = std::to_array<uint8_t>({0xff, 0xff});

struct [[nodiscard]] ServoId{
    uint8_t count;
};


struct [[nodiscard]] Regs{
    using RegAddr = uint8_t;

    struct [[nodiscard]] R8_SoftwareVersion:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x02};
        uint8_t version;
    }DEF_R8(software_version_reg)

    struct [[nodiscard]] R8_Id:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x03};
        uint8_t id;
    }DEF_R8(id_reg)

    struct [[nodiscard]] R8_Baudrate:public Reg8<>{
        static constexpr RegAddr REG_ADDR = RegAddr{0x04};
        uint8_t baud;
    }DEF_R8(baud_reg)
};

struct [[nodiscard]] BytesFiller{
public:

    constexpr explicit BytesFiller(std::span<uint8_t> bytes):
        bytes_(bytes){;}

    constexpr ~BytesFiller(){
        if(not is_full()) __builtin_abort();
    }

    constexpr __always_inline 
    void push_byte(const uint8_t byte){
        if(pos_ >= bytes_.size()) [[unlikely]] 
            on_overflow();
        bytes_[pos_++] = byte;
    }

    constexpr __always_inline 
    void push_zero(){
        push_byte(0);
    }

    constexpr __always_inline 
    void push_zeros(size_t n){
        #pragma GCC unroll(4)
        for(size_t i = 0; i < n; i++)
            push_byte(0);
    }

    constexpr __always_inline 
    void fill_remaining(const uint8_t byte){
        const size_t n = bytes_.size() - pos_;

        #pragma GCC unroll(4)
        for(size_t i = 0; i < n; i++){
            push_byte_unchecked(byte);
        }
    }

    template<size_t Extents>
    constexpr __always_inline 
    void push_bytes(const std::span<const uint8_t, Extents> bytes){
        if(pos_ + bytes.size() > bytes_.size()) [[unlikely]]
            on_overflow();
        push_bytes_unchecked(bytes);
    }


    template<typename T>
    requires (std::is_integral_v<T>)
    constexpr __always_inline 
    void push_int(const T i_val){
        const auto bytes = std::bit_cast<std::array<uint8_t, sizeof(T)>>(i_val);
        push_bytes(std::span(bytes));
    }


    [[nodiscard]] constexpr bool is_full() const noexcept {
        return pos_ == bytes_.size();
    }
private:
    std::span<uint8_t> bytes_;
    size_t pos_ = 0;

    constexpr __always_inline 
    void push_byte_unchecked(const uint8_t byte){ 
        bytes_[pos_++] = byte;
    }

    template<size_t Extents>
    constexpr __always_inline 
    void push_bytes_unchecked(const std::span<const uint8_t, Extents> bytes){ 
        if constexpr(Extents == std::dynamic_extent){
            #pragma GCC unroll(4)
            for(size_t i = 0; i < bytes.size(); i++){
                push_byte(bytes[i]);
            }
        }else{
            #pragma GCC unroll(4)
            for(size_t i = 0; i < Extents; i++){
                push_byte(bytes[i]);
            }
        }
    }

    constexpr __always_inline void on_overflow(){
        __builtin_trap();
    }
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


    [[nodiscard]] static constexpr uint8_t calc_checksum(const ServoId id, std::span<const uint8_t> pbuf){
        const size_t len = pbuf.size() + 1;
        const uint32_t sum = id.count + len + std::accumulate(pbuf.begin(), pbuf.end(), 0);
        return uint8_t(~sum);
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
        filler.push_byte(calc_checksum(id, payload));
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
        filler.push_byte(Command(Command::Kind::WriteData).to_u8());
        filler.push_byte(addr);
        filler.push_bytes(data);
        return buf;
    }

    template<size_t N>
    static constexpr std::array<uint8_t, N + 2> make_sync_write_payload(const uint8_t addr, std::span<const uint8_t, N> data){
        std::array<uint8_t, N + 2> buf;
        auto filler = BytesFiller(buf);
        filler.push_byte(Command(Command::Kind::WriteData).to_u8());
        filler.push_byte(addr);
        filler.push_bytes(data);
        return buf;
    }

    static constexpr std::array<uint8_t, 3> make_read_data_payload(const uint8_t addr, const uint8_t len){
        std::array<uint8_t, 3> buf;
        auto filler = BytesFiller(buf);
        filler.push_byte(Command(Command::Kind::ReadData).to_u8());
        filler.push_byte(addr);
        filler.push_byte(len);
        return buf;
    }

    template<size_t N>
    static constexpr std::array<uint8_t, N + 2> make_async_write_data_payload(const uint8_t addr, std::span<const uint8_t, N> data){
        std::array<uint8_t, N + 2> buf;
        auto filler = BytesFiller(buf);
        filler.push_byte(Command(Command::Kind::AsyncWrite).to_u8());
        filler.push_byte(addr);
        filler.push_bytes(data);
        return buf;
    }

    static constexpr std::array<uint8_t, 1> make_invoke_async_payload(){
        std::array<uint8_t, 1> buf;
        buf[0] = (Command(Command::Kind::InvokeAsync).to_u8());
        return buf;
    }

    static constexpr std::array<uint8_t, 1> make_ping_payload(){
        std::array<uint8_t, 1> buf;
        buf[0] = (Command(Command::Kind::Ping).to_u8());
        return buf;
    }

    static constexpr std::array<uint8_t, 1> make_reset_payload(){
        std::array<uint8_t, 1> buf;
        buf[0] = (Command(Command::Kind::Reset).to_u8());
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