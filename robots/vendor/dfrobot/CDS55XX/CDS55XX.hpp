#pragma once

#include <array>
#include <algorithm>
#include <bits/stl_numeric.h>

#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/Errno.hpp"

namespace ymd::robots{


template<typename T ,size_t N>
class VectorOnArray{
public:
    constexpr void push_back(const T val){
        if(size_ < N){
            data_[size_++] = val;
        }else{
            __builtin_abort();
        }
    }

    constexpr void insert(T * pos, const auto beg, const auto ed){
        if(size_ + std::distance(beg, ed) <= N){
            std::copy(beg, ed, pos);
            size_ += std::distance(beg, ed);
        }else{
            __builtin_abort();
        }
    }
    constexpr auto size() const {return size_;}
    constexpr auto & get() const {
        if(size_ != N) __builtin_abort();
        return data_;
    }

    constexpr auto & operator[](const size_t idx) const {return data_[idx];}
    constexpr auto & operator[](const size_t idx) {return data_[idx];}

    constexpr std::span<const T, N> view() const {return std::span<const T, N>(data_);}

    constexpr const T * begin() const {return &data_[0];}
    constexpr const T * end() const {return &data_[size_];}

    constexpr T * begin(){return &data_[0];}
    constexpr T * end(){return &data_[size_];}
private:
    std::array<T, N> data_;
    size_t size_ = 0;
};

struct CDS55XX_Prelude {
    class Command{
    public:
        enum class Kind{
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

        constexpr uint8_t as_u8() const {return static_cast<uint8_t>(kind_);}
    private:
        Kind kind_;
    };

    static constexpr auto HEADER = std::to_array<uint8_t>({0xff, 0xff});

    class ServoId{
    public:
        explicit constexpr ServoId(uint8_t id):id_(id){}
        constexpr uint8_t as_u8() const {return id_;}
    private:
        uint8_t id_;
    };

};

struct CDS55XX_Regs:public CDS55XX_Prelude{
    using RegAddr = uint8_t;

    struct R8_SoftwareVersion:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x02;
        uint8_t version;
    }DEF_R8(software_version_reg)

    struct R8_Id:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x03;
        uint8_t id;
    }DEF_R8(id_reg)

    struct R8_Baudrate:public Reg8<>{
        static constexpr RegAddr ADDRESS = 0x04;
        uint8_t baud;
    }DEF_R8(buad_reg)
};


struct CDS55XX_MetaUtils:public CDS55XX_Prelude{
public:

    struct FrameFactory{
        template<size_t N>
        [[nodiscard]] static constexpr auto write_data(
            const ServoId id, 
            const uint8_t addr, 
            const std::span<const uint8_t, N> data
        ){
            return assembly_frame(id, [&]{return make_write_data_payload(addr, std::span(data));});
        }

        template<size_t N>
        [[nodiscard]] static constexpr auto async_write_data(
            const ServoId id, 
            const uint8_t addr, 
            const std::span<const uint8_t, N> data
        ){
            return assembly_frame(id, [&]{return make_async_write_data_payload(addr, std::span(data));});
        }
    
    
        template<typename Dummy = void>
        [[nodiscard]] static constexpr auto read_data(
            const ServoId id, 
            const uint8_t addr, 
            const uint8_t len
        ){
            return assembly_frame(id, [&]{return make_read_data_payload(addr, len);});
        }
    
        template<typename Dummy = void>
        [[nodiscard]] static constexpr auto invoke_async(
            const ServoId id
        ){
            return assembly_frame(id, [&]{return make_invoke_async_payload();});
        }
    
        template<typename Dummy = void>
        [[nodiscard]] static constexpr auto ping(
            const ServoId id
        ){
            return assembly_frame(id, [&]{return make_ping_payload();});
        }
    
        template<typename Dummy = void>
        [[nodiscard]] static constexpr auto reset(
            const ServoId id
        ){
            return assembly_frame(id, [&]{return make_reset_payload();});
        }
    };



private:
    static constexpr uint8_t calc_checksum(const ServoId id, std::span<const uint8_t> pbuf){
        const size_t len = pbuf.size() + 1;
        const uint32_t sum = id.as_u8() + len + std::accumulate(pbuf.begin(), pbuf.end(), 0);
        return uint8_t(~sum);
    }



    template<size_t N>
    static constexpr std::array<uint8_t, N + 5> assembly_payload(const ServoId id, std::span<const uint8_t, N> payload){
        VectorOnArray<uint8_t, N + 5> ret;
        ret.insert(ret.begin(), CDS55XX_Prelude::HEADER.begin(), CDS55XX_Prelude::HEADER.end());
        ret.push_back(id.as_u8());
        const auto len = payload.size() + 1;
        ret.push_back(len);
        ret.insert(ret.end(), payload.begin(), payload.end());
        ret.push_back(calc_checksum(id, payload));
        return ret.get();
    }

    template<typename Fn>
    static constexpr auto assembly_frame(const ServoId id, Fn && fn){
        const auto payload = std::forward<Fn>(fn)();
        const auto frame = assembly_payload(id, std::span(payload));
        return frame;
    }

    template<size_t N>
    static constexpr std::array<uint8_t, N + 2> make_write_data_payload(const uint8_t addr, std::span<const uint8_t, N> data){
        VectorOnArray<uint8_t, N + 2> ret;
        ret.push_back(Command(Command::Kind::WriteData).as_u8());
        ret.push_back(addr);
        ret.insert(ret.end(), data.begin(), data.end());
        return ret.get();
    }

    template<size_t N>
    static constexpr std::array<uint8_t, N + 2> make_sync_write_payload(const uint8_t addr, std::span<const uint8_t, N> data){
        VectorOnArray<uint8_t, N + 2> ret;
        ret.push_back(Command(Command::Kind::WriteData).as_u8());
        ret.push_back(addr);
        ret.insert(ret.end(), data.begin(), data.end());
        return ret.get();
    }

    static constexpr std::array<uint8_t, 3> make_read_data_payload(const uint8_t addr, const uint8_t len){
        VectorOnArray<uint8_t, 3> ret;
        ret.push_back(Command(Command::Kind::ReadData).as_u8());
        ret.push_back(addr);
        ret.push_back(len);
        return ret.get();
    }

    template<size_t N>
    static constexpr std::array<uint8_t, N + 2> make_async_write_data_payload(const uint8_t addr, std::span<const uint8_t, N> data){
        VectorOnArray<uint8_t, N + 2> ret;
        ret.push_back(Command(Command::Kind::AsyncWrite).as_u8());
        ret.push_back(addr);
        ret.insert(ret.end(), data.begin(), data.end());
        return ret.get();
    }

    static constexpr std::array<uint8_t, 1> make_invoke_async_payload(){
        VectorOnArray<uint8_t, 1> ret;
        ret.push_back(Command(Command::Kind::InvokeAsync).as_u8());
        return ret.get();
    }

    static constexpr std::array<uint8_t, 1> make_ping_payload(){
        VectorOnArray<uint8_t, 1> ret;
        ret.push_back(Command(Command::Kind::Ping).as_u8());
        return ret.get();
    }

    static constexpr std::array<uint8_t, 1> make_reset_payload(){
        VectorOnArray<uint8_t, 1> ret;
        ret.push_back(Command(Command::Kind::Reset).as_u8());
        return ret.get();
    }
    // static constexpr void static_test(){

    //     const auto find_different = [](std::span<const uint8_t> lhs, std::span<const uint8_t> rhs) -> int{
    //         for(size_t i = 0; i < lhs.size(); ++i){
    //             if(lhs[i] != rhs[i]){
    //                 return i;
    //             }
    //         }
    //         return -1;
    //     };





    //     // constexpr auto payload = make_write_data_payload(addr, std::span(paras));

    //     {
    //         constexpr auto id = ServoId(0xfe);
    //         constexpr auto addr = 0x03;

    //         constexpr auto paras = std::array<uint8_t,1>{0x01};
    //         constexpr auto frame = FrameFactory::write_data(id, addr, std::span(paras));
    //         constexpr auto expected_frame = std::to_array<uint8_t>({
    //             0xff, 0xff, 0xfe, 0x04, 0x03, 0x03, 0x01, 0xF6
    //         });

    //         static_assert(find_different(std::span(frame), std::span(expected_frame)) < 0);

    //         // static_assert(frame[0] == 0xFF);
    //         // static_assert(frame[1] == 0xFF);
    //         // static_assert(frame[2] == 0xFe);
    //         // static_assert(frame[3] == 0x04);
    //         // static_assert(frame[4] == 0x03);
    //         // static_assert(frame[5] == 0x03);
    //         // static_assert(frame[6] == 0x01);
    //         // static_assert(frame[7] == 0xF6);
    //     }

    //     {
    //         constexpr auto id = ServoId(0x01);
    //         constexpr auto frame = FrameFactory::read_data(id, 0x2b, 0x01);
    //         constexpr auto expected_frame = std::to_array<uint8_t>({
    //             0XFF,0XFF,0X01,0X04,0X02,0X2B,0X01,0XCC
    //         });

    //         constexpr auto i = find_different(std::span(frame), std::span(expected_frame));
    //         static_assert( i< 0);
    //     }

    //     {
    //         constexpr auto id = ServoId(0x01);
    //         constexpr auto frame = FrameFactory::ping(id);
    //         constexpr auto expected_frame = std::to_array<uint8_t>({
    //             0XFF,0XFF,0X01,0X02,0X01,0XFB
    //         });
    //         constexpr auto i = find_different(std::span(frame), std::span(expected_frame));
    //         static_assert( i< 0);
    //     }

    //     {
    //         constexpr auto id = ServoId(0x00);
    //         constexpr auto frame = FrameFactory::reset(id);
    //         constexpr auto expected_frame = std::to_array<uint8_t>({
    //             0XFF,0XFF,0X00,0X02,0X06,0XF7
    //         });
    //         constexpr auto i = find_different(std::span(frame), std::span(expected_frame));
    //         static_assert( i< 0);

    //     }
    // }
};

class CDS55XX_Phy:public CDS55XX_Prelude{

    template<typename T>
    struct OperWrapper{
        T value;
    };

    class Oper{
        template<size_t N>
        [[nodiscard]] constexpr auto write_data(const uint8_t addr, const std::array<uint8_t, N>data) const {
            return make_oper(CDS55XX_MetaUtils::FrameFactory::write_data(id_, addr, std::span(data)));
        }
    private:
        ServoId id_;
    };

    const auto operate() const {
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