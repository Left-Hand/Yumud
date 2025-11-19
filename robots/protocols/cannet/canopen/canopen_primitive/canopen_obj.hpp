#pragma once

#include "core/string/string_view.hpp"
#include "canopen_sdo_primitive.hpp"

namespace ymd::canopen::primitive{


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