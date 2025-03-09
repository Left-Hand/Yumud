#pragma once

#include "sys/core/platform.h"

#include "hal/gpio/gpio.hpp"
#include "hal/gpio/GpioTag.hpp"
#include "hal/gpio/port.hpp"

#include "ral/ch32/ch32v203_opa_def.hpp"

namespace ymd::hal{

//OpaIndex: 运放序号 1/2
//TChannel: 通道序号 0:Output -1:Neg 1:Pos
//TMapping: 布局序号 0/1
template<uint8_t TOpaIndex, int8_t TChannel, uint8_t TMapping>
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

template<uint8_t TOpaIndex>
class Opa_t{
private:
    static constexpr uint8_t index = TOpaIndex;
    static_assert((index >= 1) and (index <= 4), "Opa");



public:
    template<uint8_t NegMap, uint8_t PosMap, uint8_t OutMap>
    static void remap(){
        static_assert((NegMap >= 0) and (NegMap <= 1), "Opa");
        static_assert((PosMap >= 0) and (PosMap <= 1), "Opa");
        static_assert((OutMap >= 0) and (OutMap <= 1), "Opa");

        // using NegIoTag = OpaGpioMapping<TOpaIndex, -1, NegMap>::IoTag;
        // using PosIoTag = OpaGpioMapping<TOpaIndex,  1, PosMap>::IoTag;
        // using OutIoTag = OpaGpioMapping<TOpaIndex,  0, OutMap>::IoTag;

        Gpio::reflect<GpioTags::PortSource::PA, GpioTags::PinSource::_0>().inana();
        Gpio::reflect<GpioTags::PortSource::PA, GpioTags::PinSource::_0>().inana();
        Gpio::reflect<GpioTags::PortSource::PA, GpioTags::PinSource::_0>().inana();

        // Gpio::reflect<NegIoTag::source, NegIoTag::pin>().inana();
        // Gpio::reflect<PosIoTag::source, PosIoTag::pin>().inana();
        // Gpio::reflect<OutIoTag::source, OutIoTag::pin>().inana();
    }

    template<uint8_t NegMap, uint8_t PosMap, uint8_t OutMap>
    static void init(){
        using chip::OPA_Inst;

        remap<NegMap, PosMap, OutMap>();
        OPA_Inst->enable(index, true);
        OPA_Inst->select_neg(index, NegMap);
        OPA_Inst->select_pos(index, PosMap);
        OPA_Inst->select_out(index, OutMap);
    }
};

#ifdef ENABLE_OPA1
extern Opa_t<1> opa1;
#endif

#ifdef ENABLE_OPA2
extern Opa_t<2> opa2;
#endif

}