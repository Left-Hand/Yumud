#pragma once

#include "primitive/can/can_enum.hpp"
#include "primitive/can/can_id.hpp"

namespace ymd::hal{

namespace details{

//这个类与平台有关 相关标准在SXX32的can文档中定义
struct [[nodiscard]]SXX32_CanIdentifier{

    template<details::is_canid ID>
    static constexpr SXX32_CanIdentifier from(
        const ID id,
        const CanRtr rmt
    ){
        if constexpr(std::is_same_v<ID, CanStdId>){
            return from_std_id(id, rmt);
        }else{
            return from_ext_id(id, rmt);
        }
    }

    static constexpr SXX32_CanIdentifier from_bits(uint32_t id_bits){
        return std::bit_cast<SXX32_CanIdentifier>(id_bits);
    }

    [[nodiscard]] __always_inline constexpr uint32_t as_bits() const{
        return std::bit_cast<uint32_t>(*this);
    }

    [[nodiscard]] __always_inline constexpr bool is_extended() const{
        return is_ext_;
    }

    [[nodiscard]] __always_inline constexpr bool is_standard() const{
        return !is_extended();
    }

    [[nodiscard]] __always_inline constexpr bool is_remote() const {
        return is_remote_;
    }

    [[nodiscard]] __always_inline constexpr uint32_t id_u32() const {
        if(is_ext_)
            return ext_id_;
        else
            return ext_id_ >> (29-11);
    }

    const uint32_t __resv__:1 = 1;
    
    //是否为远程帧
    uint32_t is_remote_:1;
    
    //是否为扩展帧
    uint32_t is_ext_:1;
    uint32_t ext_id_:29;
private:
    static constexpr SXX32_CanIdentifier from_std_id(
        const CanStdId id, 
        const CanRtr is_remote
    ){
        return SXX32_CanIdentifier{
            .is_remote_ = (is_remote == CanRtr::Remote), 
            .is_ext_ = false, 
            .ext_id_ = uint32_t(id.to_u11()) << 18
        };
    }

    static constexpr SXX32_CanIdentifier from_ext_id(
        const CanExtId id, 
        const CanRtr rtr
    ){
        return SXX32_CanIdentifier{
            .is_remote_ = (rtr == CanRtr::Remote), 
            .is_ext_ = true, 
            .ext_id_ = id.to_u29()
        };
    }
};
static_assert(sizeof(SXX32_CanIdentifier) == 4);

#if 0
struct [[nodiscard]] Can2B_Payload{
public:
    [[nodiscard]] __always_inline constexpr uint8_t * data() {return buf_.data();}
    [[nodiscard]] __always_inline constexpr uint8_t * begin() {return buf_.begin();}

    [[nodiscard]] __always_inline constexpr const uint8_t * data() const {return buf_.data();}
    [[nodiscard]] __always_inline constexpr const uint8_t * begin() const {return buf_.begin();}

    [[nodiscard]] __always_inline constexpr uint8_t size() const {return length_;}
    [[nodiscard]] __always_inline constexpr uint8_t operator[](uint8_t i) const {return buf_[i];}
private:
    std::array<uint8_t, 8> buf_;
    uint8_t length_;

    static constexpr Can2B_Payload from_bytes(const std::span<const uint8_t> bytes){
        Can2B_Payload ret;
        std::copy(bytes.begin(), bytes.end(), ret.begin());
        ret.length_ = bytes.size();
        return ret;
    }

    friend class CanClassicMsg;
};

static_assert(sizeof(Can2B_Payload) == 8);

#endif

}
}