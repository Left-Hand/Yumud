#pragma once

#include <cstdint>
#include <span>




namespace ymd::usb{

// Standard USB Request Types (bmRequestType)
enum class [[nodiscard]] RequestType : uint8_t {
    // Request is a USB standard request. Usually handled by UsbDevice.
    Standard = 0,
    // Request is intended for a USB class.
    Class = 1,
    // Request is vendor-specific.
    Vendor = 2,
    // Reserved.
    Reserved = 3,
};


// USB Request Recipients (bmRequestType)
enum class [[nodiscard]] Recipient:uint8_t {
    Device = 0,
    Interface = 1,
    Endpoint = 2,
    Other = 3,
    Reserved = 4,
};


struct [[nodiscard]] Request {
    using Self = Request;

    Direction direction;
    RequestType request_type;
    Recipient recipient;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;

    /// Standard USB control request Get Status
    static constexpr uint8_t GET_STATUS = 0;

    /// Standard USB control request Clear Feature
    static constexpr uint8_t CLEAR_FEATURE = 1;

    /// Standard USB control request Set Feature
    static constexpr uint8_t SET_FEATURE = 3;

    /// Standard USB control request Set Address
    static constexpr uint8_t SET_ADDRESS = 5;

    /// Standard USB control request Get Descriptor
    static constexpr uint8_t GET_DESCRIPTOR = 6;

    /// Standard USB control request Set Descriptor
    static constexpr uint8_t SET_DESCRIPTOR = 7;

    /// Standard USB control request Get Configuration
    static constexpr uint8_t GET_CONFIGURATION = 8;

    /// Standard USB control request Set Configuration
    static constexpr uint8_t SET_CONFIGURATION = 9;

    /// Standard USB control request Get Interface
    static constexpr uint8_t GET_INTERFACE = 10;

    /// Standard USB control request Set Interface
    static constexpr uint8_t SET_INTERFACE = 11;

    /// Standard USB control request Synch Frame
    static constexpr uint8_t SYNCH_FRAME = 12;

    /// Standard USB feature Endpoint Halt for Set/Clear Feature
    static constexpr uint16_t FEATURE_ENDPOINT_HALT6 = 0;

    /// Standard USB feature Device Remote Wakeup for Set/Clear Feature
    static constexpr uint16_t FEATURE_DEVICE_REMOTE_WAKEUP6 = 1;

    static constexpr Self from_bytes(const std::span<const uint8_t, 8> buf){
        const uint8_t rt = buf[0];
        const uint8_t recipient = rt & 0b11111;

        return Self{
            .direction = (rt & 0b10000000) ? Direction::In : Direction::Out,
            .request_type = std::bit_cast<RequestType>(static_cast<uint8_t>((rt >> 5) & 0b11)),
            .recipient = [&]{
                if(recipient <= 3){
                    return std::bit_cast<Recipient>(static_cast<uint8_t>(recipient));
                }else{
                    return Recipient::Reserved;
                }
            }(),
            .request = buf[1],
            .value = static_cast<uint16_t>(buf[2] | (buf[3] << 8)),
            .index = static_cast<uint16_t>(buf[4] | (buf[5] << 8)),
            .length = static_cast<uint16_t>(buf[6] | (buf[7] << 8)),
        };
    }
};

struct [[nodiscard]] OutResponse{
    using Self = OutResponse;
    static constexpr OutResponse from_accepted() { return OutResponse{true}; }
    static constexpr OutResponse from_rejected() { return OutResponse{false}; }
    [[nodiscard]] constexpr bool is_accepted() const { return is_accepted_; }
    [[nodiscard]] constexpr bool is_rejected() const { return !is_accepted_; }  
private:
    bool is_accepted_;

    explicit constexpr OutResponse(bool is_accepted) : is_accepted_(is_accepted) {}
};


struct [[nodiscard]] InResponse{
    using Self = InResponse;

    static constexpr InResponse from_accepted(std::span<const uint8_t> bytes){
        return InResponse{bytes.data(), bytes.size()};
    }

    static constexpr InResponse from_rejected(){
        return InResponse{nullptr, 0};
    }

    [[nodiscard]] constexpr size_t size() const { return length_; }
    [[nodiscard]] constexpr std::span<const uint8_t> bytes() const { 
        return std::span(p_bytes_, length_); }

    [[nodiscard]] constexpr bool is_accepted() const { return p_bytes_ != nullptr; }
private:
    const uint8_t * p_bytes_;
    size_t length_;

    constexpr explicit InResponse(const uint8_t *p_bytes, size_t length)    
        : p_bytes_(p_bytes), length_(length) {}
};




}

namespace ymd::usb{




// Standard USB Transfer Types
enum class [[nodiscard]] TransferType : uint8_t {
    Control = 0,
    Isochronous = 1,
    Bulk = 2,
    Interrupt = 3,
};

// USB Standard Request Codes (Table 9-4)
enum class [[nodiscard]] StandardRequest : uint8_t {
    GetStatus = 0,
    ClearFeature = 1,
    SetFeature = 3,
    SetAddress = 5,
    GetDescriptor = 6,
    SetDescriptor = 7,
    GetConfiguration = 8,
    SetConfiguration = 9,
    GetInterface = 10,
    SetInterface = 11,
    SynchFrame = 12,
    SetSel = 48,
    SetIsochronousDelay = 49,
};

// USB Feature Selectors (Table 9-6)
enum class [[nodiscard]] FeatureSelector : uint8_t {
    EndpointHalt = 0,
    DeviceRemoteWakeup = 1,
    TestMode = 2,
};

enum class [[nodiscard]] Direction {
    Out,
    In,
};


}
