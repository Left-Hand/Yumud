#pragma once

#include "primitive/gpio/pin_source.hpp"

namespace ymd::hal::spi{

enum class Remap:uint8_t {
    _0 = 0,
    _1 = 1,
    _2 = 2,
    _3 = 3
};

template<size_t NUM_SPI_NTH, Remap REMAP>
struct Layout{
    using sclk_pin_type = void;
    using miso_pin_type = void;
    using mosi_pin_type = void;
    using hwcs_pin_type = void;
};

template<>
struct Layout<1, Remap::_0>{
    using sclk_pin_type = PinTag<PortSource::PA, PinSource::_5>;
    using miso_pin_type = PinTag<PortSource::PA, PinSource::_6>;
    using mosi_pin_type = PinTag<PortSource::PA, PinSource::_7>;
    using hwcs_pin_type = PinTag<PortSource::PA, PinSource::_4>;
};

template<>
struct Layout<1, Remap::_1>{
    using sclk_pin_type = PinTag<PortSource::PB, PinSource::_3>;
    using miso_pin_type = PinTag<PortSource::PB, PinSource::_4>;
    using mosi_pin_type = PinTag<PortSource::PB, PinSource::_5>;
    using hwcs_pin_type = PinTag<PortSource::PA, PinSource::_15>;
};

template<>
struct Layout<2, Remap::_0>{
    using sclk_pin_type = PinTag<PortSource::PB, PinSource::_13>;
    using miso_pin_type = PinTag<PortSource::PB, PinSource::_14>;
    using mosi_pin_type = PinTag<PortSource::PB, PinSource::_15>;
    using hwcs_pin_type = PinTag<PortSource::PB, PinSource::_12>;
};

template<>
struct Layout<3, Remap::_0>{
    using sclk_pin_type = PinTag<PortSource::PB, PinSource::_3>;
    using miso_pin_type = PinTag<PortSource::PB, PinSource::_4>;
    using mosi_pin_type = PinTag<PortSource::PB, PinSource::_5>;
    using hwcs_pin_type = PinTag<PortSource::PA, PinSource::_15>;
};

template<>
struct Layout<3, Remap::_1>{
    using sclk_pin_type = PinTag<PortSource::PC, PinSource::_10>;
    using miso_pin_type = PinTag<PortSource::PC, PinSource::_11>;
    using mosi_pin_type = PinTag<PortSource::PC, PinSource::_12>;
    using hwcs_pin_type = PinTag<PortSource::PA, PinSource::_4>;
};

template<size_t NUM_SPI_NTH, Remap remap>
using sclk_pin_t = typename Layout<NUM_SPI_NTH, remap>::sclk_pin_type;

template<size_t NUM_SPI_NTH, Remap remap>
using miso_pin_t = typename Layout<NUM_SPI_NTH, remap>::miso_pin_type;

template<size_t NUM_SPI_NTH, Remap remap>
using mosi_pin_t = typename Layout<NUM_SPI_NTH, remap>::mosi_pin_type;

template<size_t NUM_SPI_NTH, Remap remap>
using hwcs_pin_t = typename Layout<NUM_SPI_NTH, remap>::hwcs_pin_type;

}

namespace ymd::hal{
using SpiRemap = spi::Remap;

static constexpr SpiRemap SPI1_REMAP_PA5_PA6_PA7_PA4 = SpiRemap::_0;
static constexpr SpiRemap SPI1_REMAP_PB3_PB4_PB5_PA15 = SpiRemap::_1;
static constexpr SpiRemap SPI2_REMAP_PB13_PB14_PB15_PB12 = SpiRemap::_0;
static constexpr SpiRemap SPI3_REMAP_PB3_PB4_PB5_PA15 = SpiRemap::_0;
static constexpr SpiRemap SPI3_REMAP_PC10_PC11_PC12_PA4 = SpiRemap::_1;
}
