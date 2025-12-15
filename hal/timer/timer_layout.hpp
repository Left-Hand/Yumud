#pragma once

#include "primitive/gpio/pin_source.hpp"


#define TIM1_CH1_DMA_CH hal::dma1_ch2
#define TIM1_CH2_DMA_CH hal::dma1_ch3
#define TIM1_CH3_DMA_CH hal::dma1_ch6
#define TIM1_CH4_DMA_CH hal::dma1_ch4

#define TIM2_CH1_DMA_CH hal::dma1_ch5
#define TIM2_CH2_DMA_CH hal::dma1_ch7
#define TIM2_CH3_DMA_CH hal::dma1_ch1
#define TIM2_CH4_DMA_CH hal::dma1_ch7

#define TIM3_CH1_DMA_CH hal::dma1_ch6
#define TIM3_CH3_DMA_CH hal::dma1_ch2
#define TIM3_CH4_DMA_CH hal::dma1_ch3

#define TIM4_CH1_DMA_CH hal::dma1_ch1
#define TIM4_CH2_DMA_CH hal::dma1_ch4
#define TIM4_CH3_DMA_CH hal::dma1_ch5

#define TIM5_CH1_DMA_CH hal::dma2_ch5
#define TIM5_CH2_DMA_CH hal::dma2_ch4
#define TIM5_CH3_DMA_CH hal::dma2_ch2
#define TIM5_CH4_DMA_CH hal::dma2_ch1

#define TIM8_CH1_DMA_CH hal::dma2_ch3
#define TIM8_CH2_DMA_CH hal::dma2_ch5
#define TIM8_CH3_DMA_CH hal::dma2_ch1
#define TIM8_CH4_DMA_CH hal::dma2_ch2

#define TIM9_CH1_DMA_CH hal::dma2_ch7
#define TIM9_CH2_DMA_CH hal::dma2_ch9
#define TIM9_CH3_DMA_CH hal::dma2_ch11
#define TIM9_CH4_DMA_CH hal::dma2_ch8

#define TIM10_CH1_DMA_CH hal::dma2_ch8
#define TIM10_CH2_DMA_CH hal::dma2_ch10
#define TIM10_CH3_DMA_CH hal::dma2_ch9
#define TIM10_CH4_DMA_CH hal::dma2_ch6

#define TIM1_RM0_CH1_PIN hal::PA<8>()
#define TIM1_RM0_CH1N_PIN hal::PB<13>()

#define TIM1_RM0_CH2_PIN hal::PA<9>()
#define TIM1_RM0_CH2N_PIN hal::PB<14>()

#define TIM1_RM0_CH3_PIN hal::PA<10>()
#define TIM1_RM0_CH3N_PIN hal::PB<15>()

#define TIM1_RM0_CH4_PIN hal::PA<11>()

#define TIM1_RM1_CH1_PIN hal::PA<8>()
#define TIM1_RM1_CH1N_PIN hal::PA<7>()

#define TIM1_RM1_CH2_PIN hal::PA<9>()
#define TIM1_RM1_CH2N_PIN hal::PB<0>()

#define TIM1_RM1_CH3_PIN hal::PA<10>()
#define TIM1_RM1_CH3N_PIN hal::PB<1>()

#define TIM1_RM1_CH4_PIN hal::PA<11>()

#ifdef GPIOE_PRESENT
#define TIM1_RM3_CH1_PIN hal::PE<9>()
#define TIM1_RM3_CH1N_PIN hal::PE<8>()

#define TIM1_RM3_CH2_PIN hal::PE<11>()
#define TIM1_RM3_CH2N_PIN hal::PE<10>()

#define TIM1_RM3_CH3_PIN hal::PE<13>()
#define TIM1_RM3_CH3N_PIN hal::PE<12>()

#define TIM1_RM3_CH4_PIN hal::PE<14>()
#endif


#define TIM2_RM0_CH1_PIN hal::PA<0>()
#define TIM2_RM0_CH2_PIN hal::PA<1>()
#define TIM2_RM0_CH3_PIN hal::PA<2>()
#define TIM2_RM0_CH4_PIN hal::PA<3>()
#define TIM2_RM1_CH1_PIN hal::PA<15>()
#define TIM2_RM1_CH2_PIN hal::PB<3>()
#define TIM2_RM1_CH3_PIN hal::PA<2>()
#define TIM2_RM1_CH4_PIN hal::PA<3>()
#define TIM2_RM2_CH1_PIN hal::PA<0>()
#define TIM2_RM2_CH2_PIN hal::PA<1>()
#define TIM2_RM2_CH3_PIN hal::PB<10>()
#define TIM2_RM2_CH4_PIN hal::PB<11>()
#define TIM2_RM3_CH1_PIN hal::PA<15>()
#define TIM2_RM3_CH2_PIN hal::PB<3>()
#define TIM2_RM3_CH3_PIN hal::PB<10>()
#define TIM2_RM3_CH4_PIN hal::PB<11>()


#define TIM3_RM0_CH1_PIN hal::PA<6>()
#define TIM3_RM0_CH2_PIN hal::PA<7>()
#define TIM3_RM0_CH3_PIN hal::PB<0>()
#define TIM3_RM0_CH4_PIN hal::PB<1>()
#define TIM3_RM2_CH1_PIN hal::PB<4>()
#define TIM3_RM2_CH2_PIN hal::PB<5>()
#define TIM3_RM2_CH3_PIN hal::PB<0>()
#define TIM3_RM2_CH4_PIN hal::PB<1>()



#define TIM4_RM0_CH1_PIN hal::PB<6>()
#define TIM4_RM0_CH2_PIN hal::PB<7>()
#define TIM4_RM0_CH3_PIN hal::PB<8>()
#define TIM4_RM0_CH4_PIN hal::PB<9>()
#define TIM4_RM1_CH1_PIN hal::PD<12>()
#define TIM4_RM1_CH2_PIN hal::PD<13>()
#define TIM4_RM1_CH3_PIN hal::PD<14>()
#define TIM4_RM1_CH4_PIN hal::PD<15>()


#define TIM5_RM0_CH1_PIN hal::PA<0>()
#define TIM5_RM0_CH2_PIN hal::PA<1>()
#define TIM5_RM0_CH3_PIN hal::PA<2>()
#define TIM5_RM0_CH4_PIN hal::PA<3>()


#define TIM8_RM0_CH1_PIN hal::PC<6>()
#define TIM8_RM0_CH1N_PIN hal::PA<7>()

#define TIM8_RM0_CH2_PIN hal::PC<7>()
#define TIM8_RM0_CH2N_PIN hal::PB<0>()

#define TIM8_RM0_CH3_PIN hal::PC<8>()
#define TIM8_RM0_CH3N_PIN hal::PB<1>()

#define TIM8_RM0_CH4_PIN hal::PC<9>()
#define TIM8_RM1_CH1_PIN hal::PB<6>()
#define TIM8_RM1_CH1N_PIN hal::PA<13>()

#define TIM8_RM1_CH2_PIN hal::PB<7>()
#define TIM8_RM1_CH2N_PIN hal::PA<14>()

#define TIM8_RM1_CH3_PIN hal::PB<8>()
#define TIM8_RM1_CH3N_PIN hal::PA<15>()

#define TIM8_RM1_CH4_PIN hal::PC<13>()


#define TIM9_RM0_CH1_PIN hal::PA<2>()
#define TIM9_RM0_CH1N_PIN hal::PC<0>()

#define TIM9_RM0_CH2_PIN hal::PA<3>()
#define TIM9_RM0_CH2N_PIN hal::PC<1>()

#define TIM9_RM0_CH3_PIN hal::PA<4>()
#define TIM9_RM0_CH3N_PIN hal::PC<2>()

#define TIM9_RM0_CH4_PIN hal::PC<4>()
#define TIM9_RM1_CH1_PIN hal::PA<2>()
#define TIM9_RM1_CH1N_PIN hal::PB<0>()

#define TIM9_RM1_CH2_PIN hal::PA<3>()
#define TIM9_RM1_CH2N_PIN hal::PB<1>()

#define TIM9_RM1_CH3_PIN hal::PA<4>()
#define TIM9_RM1_CH3N_PIN hal::PB<2>()

#define TIM9_RM1_CH4_PIN hal::PC<14>()

#ifdef GPIOD_PRESENT
#define TIM9_RM2_CH1_PIN hal::PD<9>()
#define TIM9_RM2_CH1N_PIN hal::PD<8>()

#define TIM9_RM2_CH2_PIN hal::PD<11>()
#define TIM9_RM2_CH2N_PIN hal::PD<10>()

#define TIM9_RM2_CH3_PIN hal::PD<13>()
#define TIM9_RM2_CH3N_PIN hal::PD<12>()

#define TIM9_RM2_CH4_PIN hal::PD<15>()
#define TIM9_RM3_CH1_PIN hal::PD<9>()
#define TIM9_RM3_CH1N_PIN hal::PD<8>()

#define TIM9_RM3_CH2_PIN hal::PD<11>()
#define TIM9_RM3_CH2N_PIN hal::PD<10>()

#define TIM9_RM3_CH3_PIN hal::PD<13>()
#define TIM9_RM3_CH3N_PIN hal::PD<12>()

#define TIM9_RM3_CH4_PIN hal::PD<15>()
#endif

#define TIM10_RM0_CH1_PIN hal::PB<8>()
#define TIM10_RM0_CH1N_PIN hal::PA<12>()

#define TIM10_RM0_CH2_PIN hal::PB<9>()
#define TIM10_RM0_CH2N_PIN hal::PA<13>()

#define TIM10_RM0_CH3_PIN hal::PC<3>()
#define TIM10_RM0_CH3N_PIN hal::PA<14>()

#define TIM10_RM0_CH4_PIN hal::PC<11>()

#define TIM10_RM1_CH1_PIN hal::PB<3>()
#define TIM10_RM1_CH1N_PIN hal::PA<5>()

#define TIM10_RM1_CH2_PIN hal::PB<4>()
#define TIM10_RM1_CH2N_PIN hal::PA<6>()

#define TIM10_RM1_CH3_PIN hal::PB<5>()
#define TIM10_RM1_CH3N_PIN hal::PA<7>()

#define TIM10_RM1_CH4_PIN hal::PC<14>()

#define TIM10_RM2_CH1_PIN hal::PD<1>()

#ifdef GPIOD_PRESENT
#define TIM10_RM2_CH2_PIN hal::PD<3>()
#define TIM10_RM2_CH3_PIN hal::PD<5>()
#define TIM10_RM2_CH4_PIN hal::PD<7>()
#define TIM10_RM3_CH1_PIN hal::PD<1>()
#define TIM10_RM3_CH2_PIN hal::PD<3>()
#define TIM10_RM3_CH3_PIN hal::PD<5>()
#define TIM10_RM3_CH4_PIN hal::PD<7>()
#endif
#ifdef GPIOE_PRESENT
#define TIM10_RM2_CH1N_PIN hal::PE<3>()
#define TIM10_RM2_CH2N_PIN hal::PE<4>()
#define TIM10_RM2_CH3N_PIN hal::PE<5>()
#define TIM10_RM3_CH1N_PIN hal::PE<3>()
#define TIM10_RM3_CH2N_PIN hal::PE<4>()
#define TIM10_RM3_CH3N_PIN hal::PE<5>()
#endif

namespace ymd::hal::timer{

enum class Remap:uint8_t{
    _0 = 0,
    _1 = 1,
    _2 = 2,
    _3 = 3
};

template<size_t NUM_TIMER_NTH, Remap REMAP>
struct Layout{
    using ch1_pin_type = void;
    using ch2_pin_type = void;
    using ch3_pin_type = void;
    using ch4_pin_type = void;
    using ch1n_pin_type = void;
    using ch2n_pin_type = void;
    using ch3n_pin_type = void;
};

// TIM1 Remap 0
template<>
struct Layout<1, Remap::_0> {
    using ch1_pin_type = PinTag<PortSource::PA, PinSource::_8>;
    using ch1n_pin_type = PinTag<PortSource::PB, PinSource::_13>;
    using ch2_pin_type = PinTag<PortSource::PA, PinSource::_9>;
    using ch2n_pin_type = PinTag<PortSource::PB, PinSource::_14>;
    using ch3_pin_type = PinTag<PortSource::PA, PinSource::_10>;
    using ch3n_pin_type = PinTag<PortSource::PB, PinSource::_15>;
    using ch4_pin_type = PinTag<PortSource::PA, PinSource::_11>;
};

// TIM1 Remap 1
template<>
struct Layout<1, Remap::_1> {
    using ch1_pin_type = PinTag<PortSource::PA, PinSource::_8>;
    using ch2_pin_type = PinTag<PortSource::PA, PinSource::_9>;
    using ch3_pin_type = PinTag<PortSource::PA, PinSource::_10>;
    using ch4_pin_type = PinTag<PortSource::PA, PinSource::_11>;

    using ch1n_pin_type = PinTag<PortSource::PA, PinSource::_7>;
    using ch2n_pin_type = PinTag<PortSource::PB, PinSource::_0>;
    using ch3n_pin_type = PinTag<PortSource::PB, PinSource::_1>;
};

// TIM1 Remap 3 
template<>
struct Layout<1, Remap::_3> {
    using ch1_pin_type = PinTag<PortSource::PE, PinSource::_9>;
    using ch1n_pin_type = PinTag<PortSource::PE, PinSource::_8>;
    using ch2_pin_type = PinTag<PortSource::PE, PinSource::_11>;
    using ch2n_pin_type = PinTag<PortSource::PE, PinSource::_10>;
    using ch3_pin_type = PinTag<PortSource::PE, PinSource::_13>;
    using ch3n_pin_type = PinTag<PortSource::PE, PinSource::_12>;
    using ch4_pin_type = PinTag<PortSource::PE, PinSource::_14>;
};

// TIM2 Remap 0
template<>
struct Layout<2, Remap::_0> {
    using ch1_pin_type = PinTag<PortSource::PA, PinSource::_0>;
    using ch2_pin_type = PinTag<PortSource::PA, PinSource::_1>;
    using ch3_pin_type = PinTag<PortSource::PA, PinSource::_2>;
    using ch4_pin_type = PinTag<PortSource::PA, PinSource::_3>;
    using ch1n_pin_type = void;
    using ch2n_pin_type = void;
    using ch3n_pin_type = void;
};

// TIM2 Remap 1
template<>
struct Layout<2, Remap::_1> {
    using ch1_pin_type = PinTag<PortSource::PA, PinSource::_15>;
    using ch2_pin_type = PinTag<PortSource::PB, PinSource::_3>;
    using ch3_pin_type = PinTag<PortSource::PA, PinSource::_2>;
    using ch4_pin_type = PinTag<PortSource::PA, PinSource::_3>;
    using ch1n_pin_type = void;
    using ch2n_pin_type = void;
    using ch3n_pin_type = void;
};

// TIM2 Remap 2
template<>
struct Layout<2, Remap::_2> {
    using ch1_pin_type = PinTag<PortSource::PA, PinSource::_0>;
    using ch2_pin_type = PinTag<PortSource::PA, PinSource::_1>;
    using ch3_pin_type = PinTag<PortSource::PB, PinSource::_10>;
    using ch4_pin_type = PinTag<PortSource::PB, PinSource::_11>;
    using ch1n_pin_type = void;
    using ch2n_pin_type = void;
    using ch3n_pin_type = void;
};

// TIM2 Remap 3
template<>
struct Layout<2, Remap::_3> {
    using ch1_pin_type = PinTag<PortSource::PA, PinSource::_15>;
    using ch2_pin_type = PinTag<PortSource::PB, PinSource::_3>;
    using ch3_pin_type = PinTag<PortSource::PB, PinSource::_10>;
    using ch4_pin_type = PinTag<PortSource::PB, PinSource::_11>;
    using ch1n_pin_type = void;
    using ch2n_pin_type = void;
    using ch3n_pin_type = void;
};

// TIM3 Remap 0
template<>
struct Layout<3, Remap::_0> {
    using ch1_pin_type = PinTag<PortSource::PA, PinSource::_6>;
    using ch2_pin_type = PinTag<PortSource::PA, PinSource::_7>;
    using ch3_pin_type = PinTag<PortSource::PB, PinSource::_0>;
    using ch4_pin_type = PinTag<PortSource::PB, PinSource::_1>;
    using ch1n_pin_type = void;
    using ch2n_pin_type = void;
    using ch3n_pin_type = void;
};

// TIM3 Remap 2
template<>
struct Layout<3, Remap::_2> {
    using ch1_pin_type = PinTag<PortSource::PB, PinSource::_4>;
    using ch2_pin_type = PinTag<PortSource::PB, PinSource::_5>;
    using ch3_pin_type = PinTag<PortSource::PB, PinSource::_0>;
    using ch4_pin_type = PinTag<PortSource::PB, PinSource::_1>;
    using ch1n_pin_type = void;
    using ch2n_pin_type = void;
    using ch3n_pin_type = void;
};

// TIM4 Remap 0
template<>
struct Layout<4, Remap::_0> {
    using ch1_pin_type = PinTag<PortSource::PB, PinSource::_6>;
    using ch2_pin_type = PinTag<PortSource::PB, PinSource::_7>;
    using ch3_pin_type = PinTag<PortSource::PB, PinSource::_8>;
    using ch4_pin_type = PinTag<PortSource::PB, PinSource::_9>;
    using ch1n_pin_type = void;
    using ch2n_pin_type = void;
    using ch3n_pin_type = void;
};

// TIM4 Remap 1
template<>
struct Layout<4, Remap::_1> {
    using ch1_pin_type = PinTag<PortSource::PD, PinSource::_12>;
    using ch2_pin_type = PinTag<PortSource::PD, PinSource::_13>;
    using ch3_pin_type = PinTag<PortSource::PD, PinSource::_14>;
    using ch4_pin_type = PinTag<PortSource::PD, PinSource::_15>;
    using ch1n_pin_type = void;
    using ch2n_pin_type = void;
    using ch3n_pin_type = void;
};

// TIM5 Remap 0
template<>
struct Layout<5, Remap::_0> {
    using ch1_pin_type = PinTag<PortSource::PA, PinSource::_0>;
    using ch2_pin_type = PinTag<PortSource::PA, PinSource::_1>;
    using ch3_pin_type = PinTag<PortSource::PA, PinSource::_2>;
    using ch4_pin_type = PinTag<PortSource::PA, PinSource::_3>;
    using ch1n_pin_type = void;
    using ch2n_pin_type = void;
    using ch3n_pin_type = void;
};

// TIM8 Remap 0
template<>
struct Layout<8, Remap::_0> {
    using ch1_pin_type = PinTag<PortSource::PC, PinSource::_6>;
    using ch1n_pin_type = PinTag<PortSource::PA, PinSource::_7>;
    using ch2_pin_type = PinTag<PortSource::PC, PinSource::_7>;
    using ch2n_pin_type = PinTag<PortSource::PB, PinSource::_0>;
    using ch3_pin_type = PinTag<PortSource::PC, PinSource::_8>;
    using ch3n_pin_type = PinTag<PortSource::PB, PinSource::_1>;
    using ch4_pin_type = PinTag<PortSource::PC, PinSource::_9>;
};

// TIM8 Remap 1
template<>
struct Layout<8, Remap::_1> {
    using ch1_pin_type = PinTag<PortSource::PB, PinSource::_6>;
    using ch1n_pin_type = PinTag<PortSource::PA, PinSource::_13>;
    using ch2_pin_type = PinTag<PortSource::PB, PinSource::_7>;
    using ch2n_pin_type = PinTag<PortSource::PA, PinSource::_14>;
    using ch3_pin_type = PinTag<PortSource::PB, PinSource::_8>;
    using ch3n_pin_type = PinTag<PortSource::PA, PinSource::_15>;
    using ch4_pin_type = PinTag<PortSource::PC, PinSource::_13>;
};

// TIM9 Remap 0
template<>
struct Layout<9, Remap::_0> {
    using ch1_pin_type = PinTag<PortSource::PA, PinSource::_2>;
    using ch1n_pin_type = PinTag<PortSource::PC, PinSource::_0>;
    using ch2_pin_type = PinTag<PortSource::PA, PinSource::_3>;
    using ch2n_pin_type = PinTag<PortSource::PC, PinSource::_1>;
    using ch3_pin_type = PinTag<PortSource::PA, PinSource::_4>;
    using ch3n_pin_type = PinTag<PortSource::PC, PinSource::_2>;
    using ch4_pin_type = PinTag<PortSource::PC, PinSource::_4>;
};

// TIM9 Remap 1
template<>
struct Layout<9, Remap::_1> {
    using ch1_pin_type = PinTag<PortSource::PA, PinSource::_2>;
    using ch1n_pin_type = PinTag<PortSource::PB, PinSource::_0>;
    using ch2_pin_type = PinTag<PortSource::PA, PinSource::_3>;
    using ch2n_pin_type = PinTag<PortSource::PB, PinSource::_1>;
    using ch3_pin_type = PinTag<PortSource::PA, PinSource::_4>;
    using ch3n_pin_type = PinTag<PortSource::PB, PinSource::_2>;
    using ch4_pin_type = PinTag<PortSource::PC, PinSource::_14>;
};

// TIM9 Remap 2 and 3 
template<>
struct Layout<9, Remap::_2> {
    using ch1_pin_type = PinTag<PortSource::PD, PinSource::_9>;
    using ch1n_pin_type = PinTag<PortSource::PD, PinSource::_8>;
    using ch2_pin_type = PinTag<PortSource::PD, PinSource::_11>;
    using ch2n_pin_type = PinTag<PortSource::PD, PinSource::_10>;
    using ch3_pin_type = PinTag<PortSource::PD, PinSource::_13>;
    using ch3n_pin_type = PinTag<PortSource::PD, PinSource::_12>;
    using ch4_pin_type = PinTag<PortSource::PD, PinSource::_15>;
};

template<>
struct Layout<9, Remap::_3> {
    using ch1_pin_type = PinTag<PortSource::PD, PinSource::_9>;
    using ch1n_pin_type = PinTag<PortSource::PD, PinSource::_8>;
    using ch2_pin_type = PinTag<PortSource::PD, PinSource::_11>;
    using ch2n_pin_type = PinTag<PortSource::PD, PinSource::_10>;
    using ch3_pin_type = PinTag<PortSource::PD, PinSource::_15>;
    using ch3n_pin_type = PinTag<PortSource::PD, PinSource::_12>;
    using ch4_pin_type = PinTag<PortSource::PD, PinSource::_13>;
};

// TIM10 Remap 0
template<>
struct Layout<10, Remap::_0> {
    using ch1_pin_type = PinTag<PortSource::PB, PinSource::_8>;
    using ch1n_pin_type = PinTag<PortSource::PA, PinSource::_12>;
    using ch2_pin_type = PinTag<PortSource::PB, PinSource::_9>;
    using ch2n_pin_type = PinTag<PortSource::PA, PinSource::_13>;
    using ch3_pin_type = PinTag<PortSource::PC, PinSource::_3>;
    using ch3n_pin_type = PinTag<PortSource::PA, PinSource::_14>;
    using ch4_pin_type = PinTag<PortSource::PC, PinSource::_11>;
};

// TIM10 Remap 1
template<>
struct Layout<10, Remap::_1> {
    using ch1_pin_type = PinTag<PortSource::PB, PinSource::_3>;
    using ch1n_pin_type = PinTag<PortSource::PA, PinSource::_5>;
    using ch2_pin_type = PinTag<PortSource::PB, PinSource::_4>;
    using ch2n_pin_type = PinTag<PortSource::PA, PinSource::_6>;
    using ch3_pin_type = PinTag<PortSource::PB, PinSource::_5>;
    using ch3n_pin_type = PinTag<PortSource::PA, PinSource::_7>;
    using ch4_pin_type = PinTag<PortSource::PC, PinSource::_14>;
};

// TIM10 Remap 2 and 3
template<>
struct Layout<10, Remap::_2> {
    using ch1_pin_type = PinTag<PortSource::PD, PinSource::_1>;

    using ch1n_pin_type = PinTag<PortSource::PE, PinSource::_3>;
    using ch2n_pin_type = PinTag<PortSource::PE, PinSource::_4>;
    using ch3n_pin_type = PinTag<PortSource::PE, PinSource::_5>;

    using ch2_pin_type = PinTag<PortSource::PD, PinSource::_3>;
    using ch3_pin_type = PinTag<PortSource::PD, PinSource::_5>;
    using ch4_pin_type = PinTag<PortSource::PD, PinSource::_7>;

};

template<>
struct Layout<10, Remap::_3> {
    using ch1n_pin_type = PinTag<PortSource::PE, PinSource::_3>;
    using ch2n_pin_type = PinTag<PortSource::PE, PinSource::_4>;
    using ch3n_pin_type = PinTag<PortSource::PE, PinSource::_5>;

    using ch1_pin_type = PinTag<PortSource::PD, PinSource::_1>;
    using ch2_pin_type = PinTag<PortSource::PD, PinSource::_3>;
    using ch3_pin_type = PinTag<PortSource::PD, PinSource::_5>;
    using ch4_pin_type = PinTag<PortSource::PD, PinSource::_7>;
};


template<size_t NUM_TIM_NTH, Remap REMAP>
using ch1_pin_t = typename Layout<NUM_TIM_NTH, REMAP>::ch1_pin_type;

template<size_t NUM_TIM_NTH, Remap REMAP>
using ch2_pin_t = typename Layout<NUM_TIM_NTH, REMAP>::ch2_pin_type;

template<size_t NUM_TIM_NTH, Remap REMAP>
using ch3_pin_t = typename Layout<NUM_TIM_NTH, REMAP>::ch3_pin_type;

template<size_t NUM_TIM_NTH, Remap REMAP>
using ch4_pin_t = typename Layout<NUM_TIM_NTH, REMAP>::ch4_pin_type;

template<size_t NUM_TIM_NTH, Remap REMAP>
using ch1n_pin_t = typename Layout<NUM_TIM_NTH, REMAP>::ch1n_pin_type;

template<size_t NUM_TIM_NTH, Remap REMAP>
using ch2n_pin_t = typename Layout<NUM_TIM_NTH, REMAP>::ch2n_pin_type;

template<size_t NUM_TIM_NTH, Remap REMAP>
using ch3n_pin_t = typename Layout<NUM_TIM_NTH, REMAP>::ch3n_pin_type;

}

namespace ymd::hal{
using TimerRemap = timer::Remap;

static constexpr TimerRemap TIM1_REMAP_A8_A9_A10_A11__B13_B14_B15 = TimerRemap::_0;
static constexpr TimerRemap TIM1_REMAP_A8_A9_A10_A11__A7_B0_B1 = TimerRemap::_1;
static constexpr TimerRemap TIM1_REMAP_E9_E11_E13_E14__E8_E10_E12 = TimerRemap::_3;

static constexpr TimerRemap TIM2_REMAP_A0_A1_A2_A3 = TimerRemap::_0;
static constexpr TimerRemap TIM2_REMAP_A15_B3_A2_A3 = TimerRemap::_1;
static constexpr TimerRemap TIM2_REMAP_A0_A1_B10_B11 = TimerRemap::_2;
static constexpr TimerRemap TIM2_REMAP_A15_B3_B10_B11 = TimerRemap::_3;

static constexpr TimerRemap TIM3_REMAP_A6_A7_B0_B1 = TimerRemap::_0;
static constexpr TimerRemap TIM3_REMAP_B4_B5_B0_B1 = TimerRemap::_2;

static constexpr TimerRemap TIM4_REMAP_B6_B7_B8_B9 = TimerRemap::_0;
static constexpr TimerRemap TIM4_REMAP_D12_D13_D14_D15 = TimerRemap::_1;

static constexpr TimerRemap TIM5_REMAP_A0_A1_A2_A3 = TimerRemap::_0;

static constexpr TimerRemap TIM6_REMAP_A0_A1_A2_A3 = TimerRemap::_0;

static constexpr TimerRemap TIM8_REMAP_C6_C7_C8_C9__A7_B0_B1 = TimerRemap::_0;
static constexpr TimerRemap TIM8_REMAP_B6_B7_B8_C13__A13_A14_A15 = TimerRemap::_1;

static constexpr TimerRemap TIM9_REMAP_A2_A3_A4_C4__C0_C1_C2 = TimerRemap::_0;
static constexpr TimerRemap TIM9_REMAP_A2_A3_A4_C4__B0_B1_B2 = TimerRemap::_1;
static constexpr TimerRemap TIM9_REMAP_D9_D11_D13_D15__D8_D10_D12 = TimerRemap::_2;

static constexpr TimerRemap TIM10_REMAP_B8_B9_C3_C11__A12_A13_A14 = TimerRemap::_0;
static constexpr TimerRemap TIM10_REMAP_B3_B4_B5_C14__A5_A6_A7 = TimerRemap::_1;
static constexpr TimerRemap TIM10_REMAP_D1_D3_D5_D7__E3_E4_E5 = TimerRemap::_2;

}