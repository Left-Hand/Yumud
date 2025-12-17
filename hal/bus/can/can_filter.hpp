#pragma once

#include <initializer_list>
#include <bitset>

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

    static constexpr CanFilterConfig accept_all(){
        return from_pair(CanStdIdMaskPair::accept_all());
    }

    static constexpr CanFilterConfig whitelist(
        const std::initializer_list<details::SXX32_CanStdIdMask> & list
    ){
        CanFilterConfig ret;

        switch(list.size()){
            case 4:
                ret.mask16[1] =     std::next(list.begin(), 3) -> to_bits();
            case 3:
                ret.mask16[0] =     std::next(list.begin(), 2) -> to_bits();
            case 2:
                ret.id16[1] =       std::next(list.begin(), 1) -> to_bits();
            case 1:
                ret.id16[0] =       std::next(list.begin(), 0) -> to_bits();
                break;
            default:
                __builtin_trap();
                break;
        }
        ret.is_32bit_ = false;
        ret.is_list_mode_ = true;

        return ret;
    }

    static constexpr CanFilterConfig from_pair(
        const CanStdIdMaskPair & pair
    ){
        return from_pairs(pair, CanStdIdMaskPair::reject_all());
    }

    static constexpr CanFilterConfig from_pairs(
        const CanStdIdMaskPair & pair1, 
        const CanStdIdMaskPair & pair2
    ){
        CanFilterConfig ret;

        ret.id16[0] = pair1.id.to_bits();
        ret.id16[1] = pair2.id.to_bits();
        ret.mask16[0] = pair1.mask.to_bits();
        ret.mask16[1] = pair2.mask.to_bits();

        ret.is_32bit_ = false;
        ret.is_list_mode_ = false;

        return ret;
    }

    static constexpr CanFilterConfig from_pair(
        const CanExtIdMaskPair & pair
    ){
        CanFilterConfig ret;

        ret.id32 = pair.id.to_bits();
        ret.mask32 = pair.mask.to_bits();

        ret.is_32bit_ = true;
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
    
    bool is_32bit_;
    bool is_list_mode_;
};

class [[nodiscard]] CanFilter final{
public:

    void deinit();

    void apply(const CanFilterConfig & cfg);
private:
    void * inst_;
    uint8_t filter_nth_;
    CanFilter(void * inst, const Nth nth):
        inst_(inst), filter_nth_(nth.count()){};

    CanFilter(const CanFilter & other) = delete;
    CanFilter(CanFilter && other) = delete;

    friend class Can;
};

}