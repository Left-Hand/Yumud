#pragma once

#include <initializer_list>
#include <bitset>

#include "core/sdk.hpp"
#include "core/io/regs.hpp"
#include "primitive/can/can_enum.hpp"
#include "primitive/can/can_id.hpp"
#include "primitive/can/can_idmask.hpp"
#include "core/utils/nth.hpp"

namespace ymd{
    class StringView;
}

namespace ymd::hal{

class Can;

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