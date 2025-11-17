#pragma once

#include <initializer_list>
#include <bitset>

#include "core/sdk.hpp"
#include "core/io/regs.hpp"
#include "primitive/can/can_enum.hpp"
#include "primitive/can/can_id.hpp"
#include "core/utils/nth.hpp"

namespace ymd{
    class StringView;
}

namespace ymd::hal{

class Can;

enum class [[nodiscard]] CanRtrSpecfier:uint8_t{
    Discard,
    RemoteOnly,
    DataOnly
};

namespace details{
struct [[nodiscard]] SXX32_CanStdIdMask final{
    using id_type = CanStdId;

    #pragma pack(push, 1)
    const uint16_t __resv1__:3 = 0;
    const uint16_t __hw_wired_ide__:1 = 0;
    uint16_t is_remote:1;
    uint16_t id_u11:11;
    #pragma pack(pop)

    static constexpr SXX32_CanStdIdMask from_zero(){
        return {CanStdId(0), CanRtr::Data}; // data == 0
    }

    static constexpr  SXX32_CanStdIdMask from_full(){
        return {CanStdId(CanStdId::MAX_VALUE), CanRtr::Remote}; // remote == 1
    }

    static constexpr SXX32_CanStdIdMask from_lower_mask(const size_t len, const CanRtr rtr){
        return {CanStdId(uint16_t((1 << len) - 1)), rtr};
    }

    static constexpr SXX32_CanStdIdMask from_higher_mask(const size_t len, const CanRtr rtr){
        return {CanStdId(uint16_t(~(uint16_t(1 << len) - 1))), rtr};
    }

    [[nodiscard]] constexpr uint16_t as_bits() const{
        return std::bit_cast<uint16_t>(*this);
    }

    constexpr SXX32_CanStdIdMask(const hal::CanStdId _id, const CanRtr rtr):
        is_remote(rtr == CanRtr::Remote), id_u11(_id.to_u11()){;}

    constexpr SXX32_CanStdIdMask(const SXX32_CanStdIdMask & other) = default;
    constexpr SXX32_CanStdIdMask(SXX32_CanStdIdMask && other) = default;
private:

};

static_assert(sizeof(SXX32_CanStdIdMask) == 2);

struct [[nodiscard]] SXX32_CanExtIdMask final{
    using id_type = CanExtId;

    #pragma pack(push, 1)
    const uint32_t __resv1__:1 = 0;
    uint32_t is_remote:1;
    const uint32_t __hw_wired_ide__:1 = 1;
    uint32_t id_u29:29;
    #pragma pack(pop)

    static constexpr SXX32_CanExtIdMask from_zero(){
        return {CanExtId(0), CanRtr::Data}; // data == 0
    }

    static constexpr SXX32_CanExtIdMask from_full(){
        return {CanExtId(CanExtId::MAX_VALUE), CanRtr::Remote}; // remote == 1
    }

    static constexpr SXX32_CanExtIdMask from_lower_mask(const size_t len, const CanRtr rtr){
        return {CanExtId(uint32_t((1 << len) - 1)), rtr};
    }

    static constexpr SXX32_CanExtIdMask from_higher_mask(const size_t len, const CanRtr rtr){
        return {CanExtId(~uint32_t(uint32_t(1 << len) - 1)), rtr};
    }

    [[nodiscard]] constexpr uint32_t as_bits() const{
        return std::bit_cast<uint32_t>(*this);
    }
    constexpr SXX32_CanExtIdMask(const CanExtId _id, const CanRtr rtr):
        is_remote(uint8_t(rtr)), id_u29(_id.to_u29()){;}
    constexpr SXX32_CanExtIdMask(const SXX32_CanExtIdMask & other) = default;
    constexpr SXX32_CanExtIdMask(SXX32_CanExtIdMask && other) = default;
private:

};

static_assert(sizeof(SXX32_CanExtIdMask) == 4);
}

template<typename T>
struct [[nodiscard]] _CanIdMaskPair final{
    using id_type = typename T::id_type;
    static constexpr size_t ID_NUM_BITS = id_type::NUM_BITS;

    T id;
    T mask;

    static constexpr _CanIdMaskPair from_accept_all(){
        return _CanIdMaskPair{
            .id = T::from_zero(), 
            .mask = T::from_zero()
        };
    }

    static constexpr _CanIdMaskPair from_reject_all(){
        return _CanIdMaskPair{
            .id = T::from_full(), 
            .mask = T::from_full()
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
        const char (&str)[ID_NUM_BITS + 1],
        CanRtrSpecfier spec
    ){
        return from_str(std::span<const char, ID_NUM_BITS>(
            static_cast<const char *>(str), ID_NUM_BITS), 
            spec
        );
    }

    static constexpr Option<_CanIdMaskPair> from_str(
        std::span<const char, ID_NUM_BITS> str,
        CanRtrSpecfier spec
    ){
        // valid token: 0/1/X
        uint32_t id = 0;
        uint32_t mask = 0;
        for(size_t i = 0; i < ID_NUM_BITS; i++){
            const auto chr = str[i];
            const size_t bit_pos = ID_NUM_BITS - 1 - i; // Correct bit position mapping
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

struct [[nodiscard]] CanFilterConfig final{ 
public:
    friend class CanFilter;

    static constexpr CanFilterConfig from_accept_all(){
        return from_pair(CanStdIdMaskPair::from_accept_all());
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
                ret.id16[0] =       std::next(list.begin(), 0) -> as_bits();
                ret.id16[1] =       std::next(list.begin(), 1) -> as_bits();
                ret.mask16[0] =     std::next(list.begin(), 2) -> as_bits();
                ret.mask16[1] =     std::next(list.begin(), 3) -> as_bits();

                break;
        }
        ret.is_32_ = false;
        ret.is_list_mode_ = true;

        return ret;
    }

    static constexpr CanFilterConfig from_pair(
        const CanStdIdMaskPair & pair
    ){
        return from_pairs(pair, CanStdIdMaskPair::from_reject_all());
    }

    static constexpr CanFilterConfig from_pairs(
        const CanStdIdMaskPair & pair1, 
        const CanStdIdMaskPair & pair2
    ){
        CanFilterConfig ret;

        ret.id16[0] = pair1.id.as_bits();
        ret.id16[1] = pair2.id.as_bits();
        ret.mask16[0] = pair1.mask.as_bits();
        ret.mask16[1] = pair2.mask.as_bits();

        ret.is_32_ = false;
        ret.is_list_mode_ = false;

        return ret;
    }

    static constexpr CanFilterConfig from_pair(
        const CanExtIdMaskPair & pair
    ){
        CanFilterConfig ret;

        ret.id32 = pair.id.as_bits();
        ret.mask32 = pair.mask.as_bits();

        ret.is_32_ = true;
        ret.is_list_mode_ = false;

        return ret;
    }

    static Option<CanFilterConfig> from_str(const StringView str);

private:
    union{
        uint16_t id16[2];
        uint32_t id32;
    };
    
    union{
        uint16_t mask16[2];
        uint32_t mask32;
    };
    
    bool is_32_;
    bool is_list_mode_;
};

class [[nodiscard]] CanFilter final{
public:

    void deinit();

    void apply(const CanFilterConfig & cfg);
private:
    CAN_TypeDef * can_;
    uint8_t nth_count_;
    CanFilter(CAN_TypeDef * can, const Nth nth):
        can_(can), nth_count_(nth.count()){};

    CanFilter(const CanFilter & other) = delete;
    CanFilter(CanFilter && other) = delete;

    friend class Can;
};

}