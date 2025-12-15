#pragma once

#include "core/platform.hpp"

#include "hal/gpio/gpio.hpp"
#include "hal/gpio/gpio_tag.hpp"
#include "hal/gpio/gpio_port.hpp"

#include "ral/ch32/ch32v203_opa_def.hpp"

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



namespace ymd::hal{

template<uint8_t OpaNth>
class Opa{
private:
    static constexpr uint8_t nth = OpaNth;
    static_assert((nth >= 1) and (nth <= 4), "Opa");



public:
    template<uint8_t NegMap, uint8_t PosMap, uint8_t OutMap>
    static void remap(){
        static_assert((NegMap >= 0) and (NegMap <= 1), "Opa");
        static_assert((PosMap >= 0) and (PosMap <= 1), "Opa");
        static_assert((OutMap >= 0) and (OutMap <= 1), "Opa");

        // using NegIoTag = OpaGpioMapping<OpaNth, -1, NegMap>::IoTag;
        // using PosIoTag = OpaGpioMapping<OpaNth,  1, PosMap>::IoTag;
        // using OutIoTag = OpaGpioMapping<OpaNth,  0, OutMap>::IoTag;

        #if 0
        Gpio::reflect<GpioTags::PortSource::PA, GpioTags::PinSource::_0>().inana();
        Gpio::reflect<GpioTags::PortSource::PA, GpioTags::PinSource::_0>().inana();
        Gpio::reflect<GpioTags::PortSource::PA, GpioTags::PinSource::_0>().inana();
        #endif

        // Gpio::reflect<NegIoTag::source, NegIoTag::pin>().inana();
        // Gpio::reflect<PosIoTag::source, PosIoTag::pin>().inana();
        // Gpio::reflect<OutIoTag::source, OutIoTag::pin>().inana();
    }

    template<uint8_t NegMap, uint8_t PosMap, uint8_t OutMap>
    static void init(){

        remap<NegMap, PosMap, OutMap>();
        ral::OPA_Inst->enable(nth, EN);
        ral::OPA_Inst->select_neg(nth, NegMap);
        ral::OPA_Inst->select_pos(nth, PosMap);
        ral::OPA_Inst->select_out(nth, OutMap);
    }
};

#ifdef OPA1_PRESENT
extern Opa<1> opa1;
#endif

#ifdef OPA2_PRESENT
extern Opa<2> opa2;
#endif

}