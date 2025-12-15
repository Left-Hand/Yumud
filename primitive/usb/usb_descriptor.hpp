#pragma once

#include <cstdint>

namespace ymd::usb{


// Standard USB Descriptor Types (Table 9-5)
enum class [[nodiscard]] DescriptorType : uint8_t {
    Device = 1,
    Configuration = 2,
    String = 3,
    Interface = 4,
    Endpoint = 5,
    DeviceQualifier = 6,
    OtherSpeedConfiguration = 7,
    InterfacePower = 8,
    OTG = 9,
    Debug = 10,
    InterfaceAssociation = 11,
    BOS = 15,
    DeviceCapability = 16,
    HID = 33,
    Report = 34,
    Physical = 35,
    Hub = 41,
    SuperSpeedHub = 42,
    EndpointCompanion = 48,
};

// USB Language Identifiers (LANGIDs) - USB Spec 2.0 Table 9-15
enum class [[nodiscard]] LanguageId : uint16_t {
    English_US = 0x0409,
    English_UK = 0x0809,
    German = 0x0407,
    German_Switzerland = 0x0807,
    French = 0x040c,
    French_Canada = 0x0c0c,
    French_Switzerland = 0x100c,
    Italian = 0x0410,
    Italian_Switzerland = 0x0810,
    Japanese = 0x0411,
    Korean = 0x0412,
    Chinese_Simplified = 0x0804,
    Chinese_Traditional = 0x0404,
    Portuguese_Brazil = 0x0416,
    Portuguese_Portugal = 0x0816,
    Spanish_Spain = 0x040a,
    Spanish_Mexico = 0x080a,
    Spanish_Colombia = 0x100a,
    Russian = 0x0419,
    Dutch = 0x0413,
    Swedish = 0x041d,
    Danish = 0x0406,
    Norwegian = 0x0414,
    Finnish = 0x040b,
    Polish = 0x0415,
    Czech = 0x0405,
    Hungarian = 0x040e,
    Turkish = 0x041f,
};

enum class CapabilityType:uint8_t{
    WUSB10 = 0x01,
    USB20Extension = 0x02,
    SuperSpeed = 0x03,
    ContainerId = 0x04,
    WirelessUsb = 0x05,
    AlternateMode = 0x06,
    SuperSpeedPlus = 0x07,
    U2ExitLatency = 0x08,
    SuperSpeedPlusU2ExitLatency = 0x09,

};


// https://docs.embassy.dev/embassy-usb/git/default/descriptor/enum.SynchronizationType.html
enum class [[nodiscard]] SynchronizationType:uint8_t{
    // No synchronization is used.
    NoSynchronization = 0,

    // Unsynchronized, although sinks provide data rate feedback.
    Asynchronous = 1,

    // Synchronized using feedback or feedforward data rate information.
    Adaptive = 2,

    // Synchronized to the USB’s SOF.
    Synchronous = 3,
};

// https://docs.embassy.dev/embassy-usb/git/default/descriptor/enum.UsageType.html
enum class [[nodiscard]] UsageType:uint8_t {
    // Use the endpoint for regular data transfer.
    DataEndpoint = 0,

    // Endpoint conveys explicit feedback information for one or more data endpoints.
    FeedbackEndpoint = 1,

    // A data endpoint that also serves as an implicit feedback endpoint for one or more data endpoints.
    ImplicitFeedbackDataEndpoint = 2,

    // Reserved usage type.
    Reserved = 3,
};





}