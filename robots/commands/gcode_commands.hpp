#pragma once

#include "core/utils/serde.hpp"

namespace ymd{


namespace serde{
template<typename T>
struct SerializeIter<RawBytes, Option<T>> {
    using InnerSerializeIter = SerializeIter<RawBytes, T>;
    constexpr explicit SerializeIter(const Option<T> & may_value)
        : 
        inner_serialize_iter_(InnerSerializeIter(may_value)),
        is_some_(may_value.is_some()){;}

    constexpr bool has_next() const { return is_some_ && inner_serialize_iter_.has_next();}
    
    constexpr uint8_t next() { return is_some_ ? inner_serialize_iter_.next() : 0;}

private:
    InnerSerializeIter inner_serialize_iter_;
    bool is_some_;
};

}

namespace robots::gcode_commands{

struct RapidMove{
    bf16 x;
    bf16 y;
};

struct LinearMove{
    bf16 x;
    bf16 y;
};

struct Dwell{
    uint16_t dwell_ms;
};

struct UseInchesUnits{

};

struct UseMillimetersUnits{

};

using G0 = RapidMove;
using G1 = LinearMove;
using G20 = UseInchesUnits;
using G21 = UseMillimetersUnits;
}
}