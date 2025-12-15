#pragma once

#include "usb/usb_types.hpp"

namespace ymd::usb{


enum class [[nodiscard]] CountryCode:uint8_t {
    None = 0x00,
    Arabic = 0x01,
    Belgian = 0x02,
    Canadian_French = 0x03,
    Czech = 0x04,
    Danish = 0x05,
    Finnish = 0x06,
    French = 0x07,
    German = 0x08,
    Greek = 0x09,
    Hungarian = 0x0A,
    Italian = 0x0B,
    Dutch = 0x0C,
    Norwegian = 0x0D,
    Polish = 0x0E,
    Portuguese = 0x0F,
    Spanish = 0x10,
    Swedish = 0x11,
    SwissFrench = 0x12,
    SwissGerman = 0x13,
    SwissItalian = 0x14,
    UnitedKingdom = 0x15,
    Slovenian = 0x16,
    Slovak = 0x17,
    Estonian = 0x18,
    Lithuanian = 0x19,
    ChineseTaiwan = 0x20,
    Chinese = 0x21,
    Japanese = 0x22,
    Korean = 0x23,
};

enum class HidDescrType : uint8_t {
    Hid = 0x21,
    Report = 0x22,
    Physical = 0x23
};

struct UsbHidClassDescrPayload final{
#pragma pack(push, 1)
    const BcdUsb bcdHID;            // HID类规范版本号
    const CountryCode  bCountryCode = CountryCode::None;      // 国家代码
    const uint8_t  bNumDescriptors;   // 报告描述符的数量
    const HidDescrType  bDescriptorType2;  // 报告描述符类型，0x22表示报告描述符
    const uint16_t wDescriptorLength; // 报告描述符的长度
#pragma pack(pop)

};

}