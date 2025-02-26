#pragma once

namespace ymd::chip{

template<typename ChipFamily, typename ChipType>
struct ChipName{
    using chip_family = ChipFamily;
    using chip_type = ChipType;
};


struct CH32V{

};

struct V203_C8T6{

};

template<>
struct ChipName<CH32V, V203_C8T6>{

};



}