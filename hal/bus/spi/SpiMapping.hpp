#pragma once

#include "hal/gpio/GpioTag.hpp"
#include <cstdint>
#include <type_traits>

namespace ymd::hal{
    
template<uint8_t TIndex, uint8_t TMapping>
struct SpiGpioMapping{
    using SclkTag = void;
    using MisoTag = void;
    using MosiTag = void;
    using HwcsTag = void;
};

template<>
struct SpiGpioMapping<1,0>{
    using SclkTag = GpioTag<GpioTags::PA, GpioTags::_5>;
    using MisoTag = GpioTag<GpioTags::PA, GpioTags::_6>;
    using MosiTag = GpioTag<GpioTags::PA, GpioTags::_7>;
    using HwcsTag = GpioTag<GpioTags::PA, GpioTags::_4>;
};

template<>
struct SpiGpioMapping<1,1>{
    using SclkTag = GpioTag<GpioTags::PB, GpioTags::_3>;
    using MisoTag = GpioTag<GpioTags::PB, GpioTags::_4>;
    using MosiTag = GpioTag<GpioTags::PB, GpioTags::_5>;
    using HwcsTag = GpioTag<GpioTags::PA, GpioTags::_15>;
};

template<>
struct SpiGpioMapping<2,0>{
    using SclkTag = GpioTag<GpioTags::PB, GpioTags::_12>;
    using MisoTag = GpioTag<GpioTags::PB, GpioTags::_13>;
    using MosiTag = GpioTag<GpioTags::PB, GpioTags::_14>;
    using HwcsTag = GpioTag<GpioTags::PB, GpioTags::_15>;
};


template<uint8_t TIndex, uint8_t TMapping>
struct I2cGpioMapping{
    using SclTag = void;
    using SdaTag = void;
};


template<>
struct I2cGpioMapping<1,0>{
    using SclTag = GpioTag<GpioTags::PB, GpioTags::_6>;
    using SdaTag = GpioTag<GpioTags::PB, GpioTags::_7>;
};

template<>
struct I2cGpioMapping<1,1>{
    using SclTag = GpioTag<GpioTags::PB, GpioTags::_8>;
    using SdaTag = GpioTag<GpioTags::PB, GpioTags::_9>;
};

template<>
struct I2cGpioMapping<1,0>{
    using SclTag = GpioTag<GpioTags::PB, GpioTags::_10>;
    using SdaTag = GpioTag<GpioTags::PB, GpioTags::_11>;
};

template<uint8_t TIndex, uint8_t TMapping>
struct TimerGpioMapping{
    using Ch1Tag = void;
    using Ch2Tag = void;
    using Ch3Tag = void;
    using Ch4Tag = void;

    using Ch1nTag = void;
    using Ch2nTag = void;
    using Ch3nTag = void;
};


template<uint8_t TIndex, uint8_t TMapping>
constexpr bool _timer_remap_exists = false;

template<>
constexpr bool _timer_remap_exists<1,0> = true;

template<>
struct TimerGpioMapping<1, 0>{
    using Ch1Tag = GpioTag<GpioTags::PA, GpioTags::_8>;
    using Ch2Tag = GpioTag<GpioTags::PA, GpioTags::_9>;
    using Ch3Tag = GpioTag<GpioTags::PA, GpioTags::_10>;
    using Ch4Tag = GpioTag<GpioTags::PA, GpioTags::_11>;

    using Ch1nTag = GpioTag<GpioTags::PB, GpioTags::_13>;
    using Ch2nTag = GpioTag<GpioTags::PB, GpioTags::_14>;
    using Ch3nTag = GpioTag<GpioTags::PB, GpioTags::_15>;
};

template<>
constexpr bool _timer_remap_exists<1,1> = true;

template<>
struct TimerGpioMapping<1, 1>{
    using Ch1Tag = GpioTag<GpioTags::PA, GpioTags::_8>;
    using Ch2Tag = GpioTag<GpioTags::PA, GpioTags::_9>;
    using Ch3Tag = GpioTag<GpioTags::PA, GpioTags::_10>;
    using Ch4Tag = GpioTag<GpioTags::PA, GpioTags::_11>;

    using Ch1nTag = GpioTag<GpioTags::PA, GpioTags::_7>;
    using Ch2nTag = GpioTag<GpioTags::PB, GpioTags::_0>;
    using Ch3nTag = GpioTag<GpioTags::PB, GpioTags::_1>;
};

template<>
constexpr bool _timer_remap_exists<1,3> = false;

template<>
constexpr bool _timer_remap_exists<1,3> = true;

template<>
struct TimerGpioMapping<1, 3>{
    using Ch1Tag = GpioTag<GpioTags::PE, GpioTags::_9>;
    using Ch2Tag = GpioTag<GpioTags::PE, GpioTags::_11>;
    using Ch3Tag = GpioTag<GpioTags::PE, GpioTags::_13>;
    using Ch4Tag = GpioTag<GpioTags::PE, GpioTags::_14>;

    using Ch1nTag = GpioTag<GpioTags::PE, GpioTags::_8>;
    using Ch2nTag = GpioTag<GpioTags::PE, GpioTags::_10>;
    using Ch3nTag = GpioTag<GpioTags::PE, GpioTags::_12>;
};


template<uint8_t TIndex, uint8_t TMapping>
struct CanGpioMapping{
    using TxTag = void;
    using RxTag = void;
};

template<>
struct CanGpioMapping<1,0>{
    using TxTag = GpioTag<GpioTags::PA, GpioTags::_12>;
    using RxTag = GpioTag<GpioTags::PA, GpioTags::_11>;
};

template<>
struct CanGpioMapping<1,1>{
    using TxTag = GpioTag<GpioTags::PB, GpioTags::_9>;
    using RxTag = GpioTag<GpioTags::PB, GpioTags::_8>;
};

template<>
struct CanGpioMapping<1,3>{
    using TxTag = GpioTag<GpioTags::PD, GpioTags::_1>;
    using RxTag = GpioTag<GpioTags::PD, GpioTags::_0>;
};

template<>
struct CanGpioMapping<2,0>{
    using TxTag = GpioTag<GpioTags::PB, GpioTags::_13>;
    using RxTag = GpioTag<GpioTags::PB, GpioTags::_12>;
};

template<>
struct CanGpioMapping<2,1>{
    using TxTag = GpioTag<GpioTags::PB, GpioTags::_6>;
    using RxTag = GpioTag<GpioTags::PB, GpioTags::_5>;
};

}