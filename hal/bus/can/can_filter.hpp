#pragma once

#include <initializer_list>
#include <bitset>

#include "core/io/regs.hpp"
#include "primitive/can/can_enum.hpp"
#include "primitive/can/can_id.hpp"
#include "primitive/can/can_idmask.hpp"
#include "primitive/can/can_filter_config.hpp"
#include "core/utils/nth.hpp"

namespace ymd{
    class StringView;
}

namespace ymd::hal{

class Can;

class CanFilter;


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