#pragma once

#include "platform_spec/sxx32_can_idmask.hpp"

namespace ymd::hal{

template<typename T>
struct [[nodiscard]] _CanIdMaskPair final{
    using id_type = typename T::id_type;
    static constexpr size_t ID_NUM_BITS = id_type::NUM_BITS;

    T id;
    T mask;

    static constexpr _CanIdMaskPair accept_all(){
        return _CanIdMaskPair{
            .id = T::zero(), 
            .mask = T::zero()
        };
    }

    static constexpr _CanIdMaskPair reject_all(){
        return _CanIdMaskPair{
            .id = T::full(), 
            .mask = T::full()
        };
    }

    static constexpr _CanIdMaskPair from_parts(
        id_type id, 
        id_type mask, 
        CanRtrSpecfier spec
    ){

        return _CanIdMaskPair{
            .id = T(id, specifier_to_id_rtr(spec)), 
            .mask = T(mask, specifier_to_mask_rtr(spec))
        };
    }

    static constexpr Option<_CanIdMaskPair> from_chars(
        const char (&p_chars)[ID_NUM_BITS + 1],
        CanRtrSpecfier spec
    ){
        return from_chars(std::span<const char, ID_NUM_BITS>(
            static_cast<const char *>(p_chars), ID_NUM_BITS), 
            spec
        );
    }

    static constexpr Option<_CanIdMaskPair> from_chars(
        std::span<const char, ID_NUM_BITS> chars,
        CanRtrSpecfier spec
    ){
        // valid token: 0/1/X
        uint32_t id = 0;
        uint32_t mask = 0;
        for(size_t i = 0; i < ID_NUM_BITS; i++){
            const char chr = chars[i];
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
        if constexpr(std::is_same_v<id_type, hal::CanStdId>){
            return Some(from_parts(
                id_type::from_u11(id),
                id_type::from_u11(mask),
                spec
            ));
        }else if constexpr(std::is_same_v<id_type, hal::CanExtId>){
            return Some(from_parts(
                id_type::from_u29(id),
                id_type::from_u29(mask),
                spec
            ));
        }
    }

private:
    static constexpr CanRtr specifier_to_id_rtr(CanRtrSpecfier spec){
        switch(spec){
            case CanRtrSpecfier::RemoteOnly: return CanRtr::Remote;
            default: return CanRtr::Data;
        };
    }

    static constexpr CanRtr specifier_to_mask_rtr(CanRtrSpecfier spec){
        switch(spec){
            case CanRtrSpecfier::Discard: return CanRtr::Data;
            default: return CanRtr::Remote;
        };
    }
};

using CanStdIdMaskPair = _CanIdMaskPair<details::SXX32_CanStdIdMask>;
using CanExtIdMaskPair = _CanIdMaskPair<details::SXX32_CanExtIdMask>;


[[maybe_unused]] static constexpr auto CAN_FILTER_PAIR_STD_DATA_FRAME_ONLY = 
    hal::CanStdIdMaskPair::from_chars("xxxxxxxxxxx", hal::CanRtrSpecfier::Discard).unwrap();
    
}