#pragma once

#include "primitive/gpio/pin_source.hpp"

#define SPI1_RM0_CS_PIN hal::PA<4>()
#define SPI1_RM0_SCLK_PIN hal::PA<5>()
#define SPI1_RM0_MISO_PIN hal::PA<6>()
#define SPI1_RM0_MOSI_PIN hal::PA<7>()

#define SPI1_RM1_CS_PIN hal::PA<15>()
#define SPI1_RM1_SCLK_PIN hal::PB<3>()
#define SPI1_RM1_MISO_PIN hal::PB<4>()
#define SPI1_RM1_MOSI_PIN hal::PB<5>()

#define SPI2_RM0_CS_PIN hal::PB<12>()
#define SPI2_RM0_SCLK_PIN hal::PB<13>()
#define SPI2_RM0_MISO_PIN hal::PB<14>()
#define SPI2_RM0_MOSI_PIN hal::PB<15>()

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
}
