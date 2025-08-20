#pragma once

#include <initializer_list>
#include <bitset>

#include "core/io/regs.hpp"
#include "can_utils.hpp"
#include "can_id.hpp"

namespace ymd{
    class StringView;
}

namespace ymd::hal{

class Can;

enum class CanRtrSpecfier:uint8_t{
    Discard,
    RemoteOnly,
    DataOnly
};

namespace details{
struct SXX32_CanStdIdMask final{
    using id_type = CanStdId;

    #pragma pack(push, 1)
    const uint16_t __resv1__:3 = 0;
    const uint16_t ide:1 = 0;
    uint16_t rtr:1;
    uint16_t id:11;
    #pragma pack(pop)

    static constexpr SXX32_CanStdIdMask from_accept_all(){
        return {CanStdId(0), CanRtr::Data};
    }

    static constexpr  SXX32_CanStdIdMask from_reject_all(){
        return {CanStdId(CanStdId::MAX_VALUE), CanRtr::Remote};
    }

    static constexpr SXX32_CanStdIdMask from_ignore_high(const size_t len, const CanRtr rmt){
        return {CanStdId(uint16_t((1 << len) - 1)), rmt};
    }

    static constexpr SXX32_CanStdIdMask from_ignore_low(const size_t len, const CanRtr rmt){
        return {CanStdId(uint16_t(~(uint16_t(1 << len) - 1))), rmt};
    }

    constexpr uint16_t as_u16() const{
        return std::bit_cast<uint16_t>(*this);
    }

    constexpr SXX32_CanStdIdMask(const hal::CanStdId _id, const CanRtr rmt):
        rtr(uint8_t(rmt)), id(_id.to_u11()){;}

    constexpr SXX32_CanStdIdMask(const SXX32_CanStdIdMask & other) = default;
    constexpr SXX32_CanStdIdMask(SXX32_CanStdIdMask && other) = default;
private:

};

static_assert(sizeof(SXX32_CanStdIdMask) == 2);

struct SXX32_CanExtIdMask final{
    using id_type = CanExtId;

    #pragma pack(push, 1)
    const uint32_t __resv1__:1 = 0;
    uint32_t rtr:1;
    const uint32_t ide:1 = 1;
    uint32_t id:29;
    #pragma pack(pop)

    static constexpr SXX32_CanExtIdMask from_accept_all(){
        return {CanExtId(0), CanRtr::Data};
    }

    static constexpr SXX32_CanExtIdMask from_reject_all(){
        return {CanExtId(CanExtId::MAX_VALUE), CanRtr::Remote};
    }

    static constexpr SXX32_CanExtIdMask from_ignore_high(const size_t len, const CanRtr rmt){
        return {CanExtId(uint32_t((1 << len) - 1)), rmt};
    }

    static constexpr SXX32_CanExtIdMask from_ignore_low(const size_t len, const CanRtr rmt){
        return {CanExtId(~uint32_t(uint32_t(1 << len) - 1)), rmt};
    }

    constexpr uint32_t as_u32() const{
        return std::bit_cast<uint32_t>(*this);
    }
    constexpr SXX32_CanExtIdMask(const CanExtId _id, const CanRtr rmt):
        rtr(uint8_t(rmt)), id(_id.to_u29()){;}
    constexpr SXX32_CanExtIdMask(const SXX32_CanExtIdMask & other) = default;
    constexpr SXX32_CanExtIdMask(SXX32_CanExtIdMask && other) = default;
private:

};

static_assert(sizeof(SXX32_CanExtIdMask) == 4);
}

template<typename T>
struct _CanIdMaskPair final{
    using id_type = typename T::id_type;
    static constexpr size_t ID_LEN = id_type::LENGTH;

    T id;
    T mask;

    static constexpr _CanIdMaskPair from_accept_all(){
        return _CanIdMaskPair{
            .id = T::from_accept_all(), 
            .mask = T::from_accept_all()
        };
    }

    static constexpr _CanIdMaskPair from_reject_all(){
        return _CanIdMaskPair{
            .id = T::from_reject_all(), 
            .mask = T::from_reject_all()
        };
    }

    static constexpr _CanIdMaskPair from_id_and_mask(
        id_type id, 
        id_type mask, 
        CanRtrSpecfier spec
    ){

        return _CanIdMaskPair{
            .id = T(id, map_specifier_to_id_rtr(spec)), 
            .mask = T(mask, map_specifier_to_mask_rtr(spec))
        };
    }

    static constexpr Option<_CanIdMaskPair> from_str(
        const char (&str)[ID_LEN + 1],
        CanRtrSpecfier spec
    ){
        return from_str(std::span<const char, ID_LEN>(
            static_cast<const char *>(str), ID_LEN), 
            spec
        );
    }

    static constexpr Option<_CanIdMaskPair> from_str(
        std::span<const char, ID_LEN> str,
        CanRtrSpecfier spec
    ){
        // valid token: 0/1/X
        uint32_t id = 0;
        uint32_t mask = 0;
        for(size_t i = 0; i < ID_LEN; i++){
            const auto chr = str[i];
            const size_t bit_pos = ID_LEN - 1 - i; // Correct bit position mapping
            switch(chr){
                // Correct approach would be:
                case '0':
                    // id bit remains 0 (no action needed if id is initialized to 0)
                    mask |= (1U << bit_pos);  // Set mask bit
                    break;
                case '1':
                    id |= (1U << bit_pos);    // Set id bit
                    mask |= (1U << bit_pos);  // Set mask bit
                    break;
                case 'x':
                case 'X':
                    // Don't set either id or mask bits (wildcard)
                    break;
                default:
                    return None;
            }
        }
        return Some(from_id_and_mask(
            id_type(id),
            id_type(mask),
            spec
        ));
    }

private:
    static constexpr CanRtr map_specifier_to_id_rtr(CanRtrSpecfier spec){
        switch(spec){
            case CanRtrSpecfier::RemoteOnly: return CanRtr::Remote;
            default: return CanRtr::Data;
        };
    }

    static constexpr CanRtr map_specifier_to_mask_rtr(CanRtrSpecfier spec){
        switch(spec){
            case CanRtrSpecfier::Discard: return CanRtr::Data;
            default: return CanRtr::Remote;
        };
    }
};



using CanStdIdMaskPair = _CanIdMaskPair<details::SXX32_CanStdIdMask>;
using CanExtIdMaskPair = _CanIdMaskPair<details::SXX32_CanExtIdMask>;


class CanFilter;

struct CanFilterConfig final{ 
public:
    friend class CanFilter;

    static constexpr CanFilterConfig from_accept_all(){
        return from_pair(
            {details::SXX32_CanStdIdMask::from_accept_all(), details::SXX32_CanStdIdMask::from_accept_all()},
            {details::SXX32_CanStdIdMask::from_accept_all(), details::SXX32_CanStdIdMask::from_accept_all()}
        );
    }

    static constexpr CanFilterConfig from_whitelist(
        const std::initializer_list<details::SXX32_CanStdIdMask> & list
    ){
        CanFilterConfig ret;

        switch(list.size()){
            default:
                HALT;
                break;
            case 2:
                ret.id16[0] =       std::next(list.begin(), 0) -> as_u16();
                ret.id16[1] =       std::next(list.begin(), 1) -> as_u16();
                ret.mask16[0] =     std::next(list.begin(), 2) -> as_u16();
                ret.mask16[1] =     std::next(list.begin(), 3) -> as_u16();

                break;
        }
        ret.is32_ = false;
        ret.islist_ = true;

        return ret;
    }

    static constexpr CanFilterConfig from_pair(
        const CanStdIdMaskPair & pair
    ){
        return from_pair(pair, CanStdIdMaskPair::from_reject_all());
    }

    static constexpr CanFilterConfig from_pair(
        const CanStdIdMaskPair & pair1, 
        const CanStdIdMaskPair & pair2
    ){
        CanFilterConfig ret;

        ret.id16[0] = pair1.id.as_u16();
        ret.id16[1] = pair2.id.as_u16();
        ret.mask16[0] = pair1.mask.as_u16();
        ret.mask16[1] = pair2.mask.as_u16();

        ret.is32_ = false;
        ret.islist_ = false;

        return ret;
    }

    static Option<CanFilterConfig> from_str(const StringView str);


    // static constexpr CanFilterConfig from_list(const std::initializer_list<SXX32_CanExtIdMask> & list){
    //     switch(list.size()){
    //         default:
    //             HALT;
    //             [[fallthrough]];

    //         case 2:
    //             id32 =      list.begin() -> as_u32();
    //             mask32 =    std::next(list.begin()) -> as_u32();
    //             break;
    //     }
    //     is32_ = true;
    //     islist_ = true;

    // }


    // static constexpr CanFilterConfig from_mask(const CanExtIdMaskPair & pair){
    //     id32 = pair.id.as_u32();
    //     mask32 = pair.mask.as_u32();

    //     is32_ = true;
    //     islist_ = false;
    // }

private:
    union{
        uint16_t id16[2];
        uint32_t id32;
    };
    
    union{
        uint16_t mask16[2];
        uint32_t mask32;
    };
    
    bool is32_;
    bool islist_;
};

class CanFilter final{
public:

    void deinit();

    void apply(const CanFilterConfig & cfg);
private:
    CAN_TypeDef * can_;
    uint8_t idx_;
    CanFilter(CAN_TypeDef * can, const uint16_t idx):can_(can), idx_(idx){};
    CanFilter(const CanFilter & other) = delete;
    CanFilter(CanFilter && other) = delete;

    friend class Can;
};

}