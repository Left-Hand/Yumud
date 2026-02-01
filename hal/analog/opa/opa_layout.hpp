#include "hal/gpio/gpio_tag.hpp"



#ifdef OPA2_PRESENT
    #define OPA2_N0_GPIO hal::PB<10>()
    #define OPA2_P0_GPIO hal::PB<14>()
    #define OPA2_O0_GPIO hal::PA<2>()

    #define OPA2_N1_GPIO hal::PA<5>()
    #define OPA2_P1_GPIO hal::PA<7>()
    #define OPA2_O1_GPIO hal::PA<4>()

#endif

#ifdef OPA1_PRESENT
    #define OPA1_N0_GPIO hal::PB<11>()
    #define OPA1_P0_GPIO hal::PB<15>()
    #define OPA1_O0_GPIO hal::PA<3>()

    #define OPA1_N1_GPIO hal::PA<6>()
    #define OPA1_P1_GPIO hal::PB<0>()
    #define OPA1_O1_GPIO hal::PB<1>()
#endif

namespace ymd::hal{

//OpaNth: 运放序号 1/2
//Channel: 通道序号 0:Output -1:Neg 1:Pos
//Mapping: 布局序号 0/1
template<uint8_t OpaNth, int8_t Channel, uint8_t Mapping>
struct OpaGpioMapping{
    using IoTag = void;
};


template<>
struct OpaGpioMapping<1, -1, 0>{
    using IoTag = GpioTag<GpioTags::PB, GpioTags::_10>;
};

template<>
struct OpaGpioMapping<1, -1, 1>{
    using IoTag = GpioTag<GpioTags::PB, GpioTags::_10>;
};

template<>
struct OpaGpioMapping<1, 0, 0>{
    using IoTag = GpioTag<GpioTags::PB, GpioTags::_10>;
};

template<>
struct OpaGpioMapping<1, 0, 1>{
    using IoTag = GpioTag<GpioTags::PB, GpioTags::_10>;
};

template<>
struct OpaGpioMapping<1, 1, 0>{
    using IoTag = GpioTag<GpioTags::PB, GpioTags::_10>;
};

template<>
struct OpaGpioMapping<1, 1, 1>{
    using IoTag = GpioTag<GpioTags::PB, GpioTags::_10>;
};


template<>
struct OpaGpioMapping<2, -1, 0>{
    using IoTag = GpioTag<GpioTags::PB, GpioTags::_10>;
};

template<>
struct OpaGpioMapping<2, -1, 1>{
    using IoTag = GpioTag<GpioTags::PB, GpioTags::_10>;
};

template<>
struct OpaGpioMapping<2, 0, 0>{
    using IoTag = GpioTag<GpioTags::PB, GpioTags::_10>;
};

template<>
struct OpaGpioMapping<2, 0, 1>{
    using IoTag = GpioTag<GpioTags::PB, GpioTags::_10>;
};

template<>
struct OpaGpioMapping<2, 1, 0>{
    using IoTag = GpioTag<GpioTags::PB, GpioTags::_10>;
};

template<>
struct OpaGpioMapping<2, 1, 1>{
    using IoTag = GpioTag<GpioTags::PB, GpioTags::_10>;
};

}