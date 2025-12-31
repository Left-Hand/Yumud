#pragma once

#include "primitive/gpio/pin_source.hpp"



#define USART1_TX_DMA_CH hal::dma1_ch4
#define USART1_RX_DMA_CH hal::dma1_ch5


#define USART2_TX_DMA_CH hal::dma1_ch7
#define USART2_RX_DMA_CH hal::dma1_ch6

#define USART3_TX_DMA_CH hal::dma1_ch2
#define USART3_RX_DMA_CH hal::dma1_ch3

#define UART4_TX_DMA_CH hal::dma2_ch5
#define UART4_RX_DMA_CH hal::dma2_ch3

#define UART5_TX_DMA_CH hal::dma2_ch4
#define UART5_RX_DMA_CH hal::dma2_ch2

#define UART6_TX_DMA_CH hal::dma2_ch6
#define UART6_RX_DMA_CH hal::dma2_ch7

#define UART7_TX_DMA_CH hal::dma2_ch8
#define UART7_RX_DMA_CH hal::dma2_ch9

#define UART8_TX_DMA_CH hal::dma2_ch10
#define UART8_RX_DMA_CH hal::dma2_ch11


#if 0
#define USART1_RM0_TX_PIN hal::PA<9>()
#define USART1_RM0_RX_PIN hal::PA<10>()

#define USART1_RM1_TX_PIN hal::PB<6>()
#define USART1_RM1_RX_PIN hal::PB<7>()

#define USART2_RM0_TX_PIN hal::PA<2>()
#define USART2_RM0_RX_PIN hal::PA<3>()

#define USART2_RM1_TX_PIN hal::PD<5>()
#define USART2_RM1_RX_PIN hal::PD<6>()

#define USART3_RM0_TX_PIN hal::PB<10>()
#define USART3_RM0_RX_PIN hal::PB<11>()

#define USART3_RM1_TX_PIN hal::PC<10>()
#define USART3_RM1_RX_PIN hal::PC<11>()

#define USART3_RM2_TX_PIN hal::PA<13>()
#define USART3_RM2_RX_PIN hal::PA<14>()

#define USART3_RM3_TX_PIN hal::PD<8>()
#define USART3_RM3_RX_PIN hal::PD<9>()

#define UART4_RM0_TX_PIN hal::PC<10>()
#define UART4_RM0_RX_PIN hal::PC<11>()

#define UART4_RM1_TX_PIN hal::PB<0>()
#define UART4_RM1_RX_PIN hal::PB<1>()

#define UART4_RM2_TX_PIN hal::PE<0>()
#define UART4_RM2_RX_PIN hal::PE<1>()

#define UART4_RM3_TX_PIN hal::PE<0>()
#define UART4_RM3_RX_PIN hal::PE<1>()

#define UART5_RM0_TX_PIN hal::PC<12>()
#define UART5_RM0_RX_PIN hal::PD<2>()

#define UART5_RM1_TX_PIN hal::PB<4>()
#define UART5_RM1_RX_PIN hal::PB<5>()

#define UART6_RM0_TX_PIN hal::PC<0>()
#define UART6_RM0_RX_PIN hal::PC<1>()

#define UART6_RM1_TX_PIN hal::PB<8>()
#define UART6_RM1_RX_PIN hal::PB<9>()

#define UART6_RM2_TX_PIN hal::PE<10>()
#define UART6_RM2_RX_PIN hal::PE<11>()

#define UART6_RM3_TX_PIN hal::PE<10>()
#define UART6_RM3_RX_PIN hal::PE<11>()

#define UART7_RM0_TX_PIN hal::PC<2>()
#define UART7_RM0_RX_PIN hal::PC<3>()

#define UART7_RM1_TX_PIN hal::PA<6>()
#define UART7_RM1_RX_PIN hal::PA<7>()

#define UART7_RM2_TX_PIN hal::PE<12>()
#define UART7_RM2_RX_PIN hal::PE<13>()

#define UART7_RM3_TX_PIN hal::PE<12>()
#define UART7_RM3_RX_PIN hal::PE<13>()

#define UART8_RM0_TX_PIN hal::PC<4>()
#define UART8_RM0_RX_PIN hal::PC<5>()

#define UART8_RM1_TX_PIN hal::PA<14>()
#define UART8_RM1_RX_PIN hal::PA<15>()

#define UART8_RM2_TX_PIN hal::PE<14>()
#define UART8_RM2_RX_PIN hal::PE<15>()

#define UART8_RM3_TX_PIN hal::PE<14>()
#define UART8_RM3_RX_PIN hal::PE<15>()
#endif

namespace ymd::hal::uart {

enum class Remap : uint8_t {
    _0 = 0,
    _1 = 1,
    _2 = 2,
    _3 = 3
};

template<size_t NUM_UART_NTH, Remap REMAP>
struct Layout{
    using tx_pin_type = void;
    using rx_pin_type = void;
};

// UART1 Remap 0
template<>
struct Layout<1, Remap::_0> {
    using tx_pin_type = PinTag<PortSource::PA, PinSource::_9>;
    using rx_pin_type = PinTag<PortSource::PA, PinSource::_10>;
};

// UART1 Remap 1
template<>
struct Layout<1, Remap::_1> {
    using tx_pin_type = PinTag<PortSource::PB, PinSource::_6>;
    using rx_pin_type = PinTag<PortSource::PB, PinSource::_7>;
};

// UART2 Remap 0
template<>
struct Layout<2, Remap::_0> {
    using tx_pin_type = PinTag<PortSource::PA, PinSource::_2>;
    using rx_pin_type = PinTag<PortSource::PA, PinSource::_3>;
};

// UART2 Remap 1
template<>
struct Layout<2, Remap::_1> {
    using tx_pin_type = PinTag<PortSource::PD, PinSource::_5>;
    using rx_pin_type = PinTag<PortSource::PD, PinSource::_6>;
};

// UART3 Remap 0
template<>
struct Layout<3, Remap::_0> {
    using tx_pin_type = PinTag<PortSource::PB, PinSource::_10>;
    using rx_pin_type = PinTag<PortSource::PB, PinSource::_11>;
};

// UART3 Remap 1
template<>
struct Layout<3, Remap::_1> {
    using tx_pin_type = PinTag<PortSource::PC, PinSource::_10>;
    using rx_pin_type = PinTag<PortSource::PC, PinSource::_11>;
};

// UART3 Remap 2
template<>
struct Layout<3, Remap::_2> {
    using tx_pin_type = PinTag<PortSource::PA, PinSource::_13>;
    using rx_pin_type = PinTag<PortSource::PA, PinSource::_14>;
};

// UART3 Remap 3
template<>
struct Layout<3, Remap::_3> {
    using tx_pin_type = PinTag<PortSource::PD, PinSource::_8>;
    using rx_pin_type = PinTag<PortSource::PD, PinSource::_9>;
};

// UART4 Remap 0
template<>
struct Layout<4, Remap::_0> {
    using tx_pin_type = PinTag<PortSource::PC, PinSource::_10>;
    using rx_pin_type = PinTag<PortSource::PC, PinSource::_11>;
};

// UART4 Remap 1
template<>
struct Layout<4, Remap::_1> {
    using tx_pin_type = PinTag<PortSource::PB, PinSource::_0>;
    using rx_pin_type = PinTag<PortSource::PB, PinSource::_1>;
};

// UART4 Remap 2
template<>
struct Layout<4, Remap::_2> {
    using tx_pin_type = PinTag<PortSource::PE, PinSource::_0>;
    using rx_pin_type = PinTag<PortSource::PE, PinSource::_1>;
};

// UART4 Remap 3
template<>
struct Layout<4, Remap::_3> {
    using tx_pin_type = PinTag<PortSource::PE, PinSource::_0>;
    using rx_pin_type = PinTag<PortSource::PE, PinSource::_1>;
};

// UART5 Remap 0
template<>
struct Layout<5, Remap::_0> {
    using tx_pin_type = PinTag<PortSource::PC, PinSource::_12>;
    using rx_pin_type = PinTag<PortSource::PD, PinSource::_2>;
};

// UART5 Remap 1
template<>
struct Layout<5, Remap::_1> {
    using tx_pin_type = PinTag<PortSource::PB, PinSource::_4>;
    using rx_pin_type = PinTag<PortSource::PB, PinSource::_5>;
};

// UART6 Remap 0
template<>
struct Layout<6, Remap::_0> {
    using tx_pin_type = PinTag<PortSource::PC, PinSource::_0>;
    using rx_pin_type = PinTag<PortSource::PC, PinSource::_1>;
};

// UART6 Remap 1
template<>
struct Layout<6, Remap::_1> {
    using tx_pin_type = PinTag<PortSource::PB, PinSource::_8>;
    using rx_pin_type = PinTag<PortSource::PB, PinSource::_9>;
};

// UART6 Remap 2
template<>
struct Layout<6, Remap::_2> {
    using tx_pin_type = PinTag<PortSource::PE, PinSource::_10>;
    using rx_pin_type = PinTag<PortSource::PE, PinSource::_11>;
};

// UART6 Remap 3
template<>
struct Layout<6, Remap::_3> {
    using tx_pin_type = PinTag<PortSource::PE, PinSource::_10>;
    using rx_pin_type = PinTag<PortSource::PE, PinSource::_11>;
};

// UART7 Remap 0
template<>
struct Layout<7, Remap::_0> {
    using tx_pin_type = PinTag<PortSource::PC, PinSource::_2>;
    using rx_pin_type = PinTag<PortSource::PC, PinSource::_3>;
};

// UART7 Remap 1
template<>
struct Layout<7, Remap::_1> {
    using tx_pin_type = PinTag<PortSource::PA, PinSource::_6>;
    using rx_pin_type = PinTag<PortSource::PA, PinSource::_7>;
};

// UART7 Remap 2
template<>
struct Layout<7, Remap::_2> {
    using tx_pin_type = PinTag<PortSource::PE, PinSource::_12>;
    using rx_pin_type = PinTag<PortSource::PE, PinSource::_13>;
};

// UART7 Remap 3
template<>
struct Layout<7, Remap::_3> {
    using tx_pin_type = PinTag<PortSource::PE, PinSource::_12>;
    using rx_pin_type = PinTag<PortSource::PE, PinSource::_13>;
};

// UART8 Remap 0
template<>
struct Layout<8, Remap::_0> {
    using tx_pin_type = PinTag<PortSource::PC, PinSource::_4>;
    using rx_pin_type = PinTag<PortSource::PC, PinSource::_5>;
};

// UART8 Remap 1
template<>
struct Layout<8, Remap::_1> {
    using tx_pin_type = PinTag<PortSource::PA, PinSource::_14>;
    using rx_pin_type = PinTag<PortSource::PA, PinSource::_15>;
};

// UART8 Remap 2
template<>
struct Layout<8, Remap::_2> {
    using tx_pin_type = PinTag<PortSource::PE, PinSource::_14>;
    using rx_pin_type = PinTag<PortSource::PE, PinSource::_15>;
};

// UART8 Remap 3
template<>
struct Layout<8, Remap::_3> {
    using tx_pin_type = PinTag<PortSource::PE, PinSource::_14>;
    using rx_pin_type = PinTag<PortSource::PE, PinSource::_15>;
};

template<size_t NUM_UART_NTH, Remap REMAP>
using tx_pin_t = typename Layout<NUM_UART_NTH, REMAP>::tx_pin_type;

template<size_t NUM_UART_NTH, Remap REMAP>
using rx_pin_t = typename Layout<NUM_UART_NTH, REMAP>::rx_pin_type;



} // namespace ymd::hal::uart

namespace ymd::hal {
using UartRemap = uart::Remap;

static constexpr UartRemap USART1_REMAP_PA9_PA10 = UartRemap::_0;
static constexpr UartRemap USART1_REMAP_PB6_PB7 = UartRemap::_1;

static constexpr UartRemap USART2_REMAP_PA2_PA3 = UartRemap::_0;
static constexpr UartRemap USART2_REMAP_PD5_PD6 = UartRemap::_1;

static constexpr UartRemap USART3_REMAP_PB10_PB11 = UartRemap::_0;
static constexpr UartRemap USART3_REMAP_PC10_PC11 = UartRemap::_1;
static constexpr UartRemap USART3_REMAP_PA13_PA14 = UartRemap::_2;
static constexpr UartRemap USART3_REMAP_PD8_PD9 = UartRemap::_3;

static constexpr UartRemap UART4_REMAP_PC10_PC11 = UartRemap::_0;
static constexpr UartRemap UART4_REMAP_PB0_PB1 = UartRemap::_1;
static constexpr UartRemap UART4_REMAP_PE0_PE1 = UartRemap::_2;
static constexpr UartRemap UART4_REMAP_PE0_PE1_ALT = UartRemap::_3;

static constexpr UartRemap UART5_REMAP_PC12_PD2 = UartRemap::_0;
static constexpr UartRemap UART5_REMAP_PB4_PB5 = UartRemap::_1;

static constexpr UartRemap UART6_REMAP_PC0_PC1 = UartRemap::_0;
static constexpr UartRemap UART6_REMAP_PB8_PB9 = UartRemap::_1;
static constexpr UartRemap UART6_REMAP_PE10_PE11 = UartRemap::_2;
static constexpr UartRemap UART6_REMAP_PE10_PE11_ALT = UartRemap::_3;

static constexpr UartRemap UART7_REMAP_PC2_PC3 = UartRemap::_0;
static constexpr UartRemap UART7_REMAP_PA6_PA7 = UartRemap::_1;
static constexpr UartRemap UART7_REMAP_PE12_PE13 = UartRemap::_2;
static constexpr UartRemap UART7_REMAP_PE12_PE13_ALT = UartRemap::_3;

static constexpr UartRemap UART8_REMAP_PC4_PC5 = UartRemap::_0;
static constexpr UartRemap UART8_REMAP_PA14_PA15 = UartRemap::_1;
static constexpr UartRemap UART8_REMAP_PE14_PE15 = UartRemap::_2;
static constexpr UartRemap UART8_REMAP_PE14_PE15_ALT = UartRemap::_3;

} // namespace ymd::hal