#pragma once

#include "j1939_primitive.hpp"
#include "core/utils/as.hpp"
#include "core/stream/ostream.hpp"

//参考资料
// https://github.com/famez/J1939-Framework/blob/master/J1939/Addressing/AddressClaimFrame.cpp
// https://github.com/Open-Agriculture/AgIsoStack-rs/blob/main/src/network_management/name.rs

namespace ymd::j1939::primitive{

using IdentityNumber = uint32_t;
using ManufacturerCode = uint16_t;
using FunctionInstance = uint8_t;
using EcuInstance = uint8_t;
using Function = uint8_t;
using VehicleSystem = uint8_t;
using VehicleSystemInstance = uint8_t;
using VehicleConfiguration = uint8_t;
using IndustryGroup = uint8_t;
using AribitaryAddressCapable = uint8_t;

struct alignas(8) [[nodiscard]] EcuName final{
    using Self = EcuName;

    uint64_t bits;

    static constexpr Self from_u64(const uint64_t bits){
        return Self{bits};
    }

    static constexpr Self from_default(){
        return Self::from_u64(0xFFFFFFFFFFFFFFFF);
    }

    [[nodiscard]] uint64_t to_u64() const{
        return bits | (static_cast<uint64_t>(1ull) << 48);
    }

    [[nodiscard]] constexpr auto identity_number() const {
        return make_bitfield_proxy<0, 21, IdentityNumber>(bits);
    }

    [[nodiscard]] constexpr auto manufacturer_code() const {
        return make_bitfield_proxy<21, 32, ManufacturerCode>(bits);
    }

    [[nodiscard]] constexpr auto ecu_instance() const {
        return make_bitfield_proxy<32, 35, EcuInstance>(bits);
    }

    [[nodiscard]] constexpr auto function_instance() const {
        return make_bitfield_proxy<35, 40, FunctionInstance>(bits);
    }

    [[nodiscard]] constexpr auto function() const {
        return make_bitfield_proxy<40, 48, Function>(bits);
    }

    [[nodiscard]] constexpr auto vehicle_system() const {
        return make_bitfield_proxy<49, 56, VehicleSystem>(bits);
    }

    [[nodiscard]] constexpr auto vehicle_system_instance() const {
        return make_bitfield_proxy<56,60, VehicleSystemInstance>(bits);
    }

    [[nodiscard]] constexpr auto industry_group() const {
        return make_bitfield_proxy<60,63, IndustryGroup>(bits);
    }

    [[nodiscard]] constexpr auto arbitary_address_capable() const {
        return make_bitfield_proxy<63,64, AribitaryAddressCapable>(bits);
    }

    friend OutputStream & operator <<(OutputStream & os, const Self & self){
        return os << os.field("identity_number")(self.identity_number().get()) << os.splitter()
            << os.field("manufacturer_code")(self.manufacturer_code().get()) << os.splitter()
            << os.field("ecu_instance")(self.ecu_instance().get()) << os.splitter()
            << os.field("function_instance")(self.function_instance().get()) << os.splitter()
            << os.field("function")(self.function().get()) << os.splitter()
            << os.field("vehicle_system")(self.vehicle_system().get()) << os.splitter()
            << os.field("vehicle_system_instance")(self.vehicle_system_instance().get()) << os.splitter()
            << os.field("industry_group")(self.industry_group().get()) << os.splitter()
            << os.field("arbitary_address_capable")(self.arbitary_address_capable().get())
        ;
    }



    [[nodiscard]] hal::BxCanFrame to_can_frame(const Pdn pdn) const{
        return hal::BxCanFrame::from_parts(
            pdn.to_can_id(),
            hal::BxCanPayload::from_u64(to_u64())
        );
    }
};

static_assert(sizeof(EcuName) == 8);

}
