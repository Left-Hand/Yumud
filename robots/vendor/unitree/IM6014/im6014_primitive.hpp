#pragma once

#include <cstdint>
#include <span>

#include "core/utils/Result.hpp"

namespace ymd::robots::unitree::im6014{


struct [[nodiscard]] TorqueCode final{
    using Self = TorqueCode;
    int16_t bits;

    static constexpr Self zero(){
        return Self{0};
    }

};



struct [[nodiscard]] X2Code final{
    using Self = X2Code;
    int16_t bits;


    static constexpr Self zero(){
        return Self{0};
    }
};



struct [[nodiscard]] X1Code final{
    using Self = X1Code;
    int32_t bits;


    static constexpr Self zero(){
        return Self{0};
    }
};


struct [[nodiscard]] KpCode final{
    using Self = KpCode;
    uint16_t bits;


    static constexpr Self zero(){
        return Self{0};
    }
};


struct [[nodiscard]] KdCode final{
    using Self = KdCode;
    uint16_t bits;


    static constexpr Self zero(){
        return Self{0};
    }
};

}