#pragma once

#include "core/string/string_view.hpp"
#include "canopen_sdo_primitive.hpp"

// https://winshton.gitbooks.io/canopen-ds301-cn/content/chapter7.5.html

namespace ymd::canopen::primitive{


enum class [[nodiscard]] EntryAccessAuthority : uint8_t {
    RW = 0,
    WO = 0x01,
    RO = 0x02,
    CONST = 0x03
};

struct [[nodiscard]] EntryObjectType {
public:
    enum class [[nodiscard]] Kind:uint8_t{ 
        Null = 0x00,
        Domain = 0x02,
        Deftype = 0x05,
        Defstruct = 0x06,
        Var = 0x07,
        Array = 0x08,
        Record = 0x09
    };
};

struct [[nodiscard]] EntryDataType {
public:

    enum class [[nodiscard]] Kind:uint8_t{
        Bit = 0x01,
        I8 = 0x02,
        I16 = 0x03,
        I32 = 0x04,
        U8 = 0x05,
        U16 = 0x06,
        U32 = 0x07,
        Real32 = 0x08,
        VisibleString = 0x09,
        OctetString = 0x0A,
        UnicodeString = 0x0B
    };

    constexpr EntryDataType(Kind kind) : kind_(kind) {}

    // 判断是否为整数类型
    [[nodiscard]] constexpr bool is_int() const {return kind_ <= Kind::U32;}
    [[nodiscard]] constexpr bool is_string() const {return kind_ >= Kind::VisibleString;}

    // 获取数据类型的大小
    constexpr Option<size_t> dsize() const {
        switch(kind_){
            case Kind::Bit:     return Some(1);
            case Kind::I8:      return Some(1);
            case Kind::I16:     return Some(2);
            case Kind::I32:     return Some(4);
            case Kind::U8:      return Some(1);
            case Kind::U16:     return Some(2);
            case Kind::U32:     return Some(4);
            case Kind::Real32:      return Some(4);
            case Kind::VisibleString:   return None;
            case Kind::UnicodeString:   return None;
            case Kind::OctetString:     return None;
        }
    }

    template<typename U>
    static constexpr Item from(){
        using T = std::decay_t<U>;
        if constexpr (std::is_same_v<T, int8_t> || std::is_same_v<T, uint8_t>) {
            return uint8;
        } else if constexpr (std::is_same_v<T, int16_t> || std::is_same_v<T, uint16_t>) {
            return uint16;
        } else if constexpr (std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t>){
            return uint32;
        } else{
            static_assert(false_v<U>, "EntryDataType::from() only support int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t");
        }
    }

private:
    Item kind_;
};




struct [[nodiscard]] CoStringObj{

    StringView str;
    SdoAbortCode write(const std::span<const uint8_t> bytes){
        if(str.length() != bytes.size()) [[unlikely]]
            return SdoAbortCode::MaxLessThanMin;

        TODO();
        // memcpy(&str[0], bytes.data(), bytes.size());
        return Ok();
    } 


    SdoAbortCode read(const std::span<uint8_t> bytes) const {
        if(str.length() != bytes.size()) [[unlikely]]
            return SdoAbortCode::MaxLessThanMin;
        return Ok();
    }
};

}