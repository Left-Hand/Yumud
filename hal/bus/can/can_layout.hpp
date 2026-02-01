#pragma once

#include "primitive/gpio/pin_source.hpp"

#define CAN1_RM0_TX_PIN hal::PA<12>()
#define CAN1_RM0_RX_PIN hal::PA<11>()

#define CAN1_RM1_TX_PIN hal::PB<9>()
#define CAN1_RM1_RX_PIN hal::PB<8>()

#define CAN1_RM3_TX_PIN hal::PD<1>()
#define CAN1_RM3_RX_PIN hal::PD<0>()

#define CAN2_RM0_TX_PIN hal::PB<13>()
#define CAN2_RM0_RX_PIN hal::PB<12>()

#define CAN2_RM1_TX_PIN hal::PB<6>()
#define CAN2_RM1_RX_PIN hal::PB<5>()

namespace ymd::hal::can{

enum class [[nodiscard]] Remap{
    _0 = 0,
    _1 = 1,
    _2 = 2,
    _3 = 3,
};

template<size_t NUM_CAN_NTH, Remap REMAP>
struct [[nodiscard]] Layout{
    using tx_pin_type = void;
    using rx_pin_type = void;
};


template<>
struct [[nodiscard]] Layout<1, Remap::_0>{
    using tx_pin_type = PinTag<PortSource::PA, PinSource::_12>;
    using rx_pin_type = PinTag<PortSource::PA, PinSource::_11>;
};

template<>
struct [[nodiscard]] Layout<1, Remap::_1>{
    using tx_pin_type = PinTag<PortSource::PB, PinSource::_9>;
    using rx_pin_type = PinTag<PortSource::PB, PinSource::_8>;
};

template<>
struct [[nodiscard]] Layout<1, Remap::_2>{
    using tx_pin_type = PinTag<PortSource::PD, PinSource::_1>;
    using rx_pin_type = PinTag<PortSource::PD, PinSource::_0>;
};

template<>
struct [[nodiscard]] Layout<2, Remap::_0>{
    using tx_pin_type = PinTag<PortSource::PB, PinSource::_13>;
    using rx_pin_type = PinTag<PortSource::PB, PinSource::_12>;
};

template<>
struct [[nodiscard]] Layout<2, Remap::_1>{
    using tx_pin_type = PinTag<PortSource::PB, PinSource::_6>;
    using rx_pin_type = PinTag<PortSource::PB, PinSource::_5>;
};


template<size_t NUM_CAN_NTH, Remap REMAP>
using tx_pin_t = typename Layout<NUM_CAN_NTH, REMAP>::tx_pin_type;

template<size_t NUM_CAN_NTH, Remap REMAP>
using rx_pin_t = typename Layout<NUM_CAN_NTH, REMAP>::rx_pin_type;

}

namespace ymd::hal{
using CanRemap = can::Remap;
static constexpr CanRemap CAN1_REMAP_PA12_PA11 = CanRemap::_0;
static constexpr CanRemap CAN1_REMAP_PB9_PB8 = CanRemap::_1;
static constexpr CanRemap CAN1_REMAP_PD1_PD0 = CanRemap::_2;

static constexpr CanRemap CAN2_REMAP_PB13_PB12 = CanRemap::_0;
static constexpr CanRemap CAN2_REMAP_PB6_PB5 = CanRemap::_1;
}