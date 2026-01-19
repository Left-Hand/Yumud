#pragma once

#include <cstdint>

// https://github.com/tbs-fpv/tbs-crsf-spec/blob/main/crsf.md

namespace ymd::crsf{

static constexpr size_t MAX_PACKET_BYTES = 64;

enum class DeviceAddress:uint8_t{
    Broadcast = 0x00,
    Cloud = 0x0E,
    UsbDevice = 0x10,
    BluetoothModule = 0x12,
    WiFiReceiver = 0x13,
    VideoReceiver = 0x14,
    OSD_TBS_CORE_PNP_PRO = 0x80,
    ESC1 = 0x90,
    ESC2 = 0x91,
    ESC3 = 0x92,
    ESC4 = 0x93,
    ESC5 = 0x94,
    ESC6 = 0x95,
    ESC7 = 0x96,
    ESC8 = 0x97,
    VoltageSensor = 0xC0,
    GPS_PNP_PRO_GPS = 0xC2,
    TBS_Blackbox = 0xC4,
    FlightController = 0xC8,
    RaceTag = 0xCC,
    VTX = 0xCE,
    RemoteControl = 0xEA,
    RcReceiver = 0xEC,
    RcTransmitter = 0xEE,
};

enum class DataType:uint8_t{
    U8              = 0,  // 0x00 // deprecated
    I8              = 1,  // 0x01 // deprecated
    U16             = 2,  // 0x02 // deprecated
    I16             = 3,  // 0x03 // deprecated
    U32             = 4,  // 0x04 // deprecated
    I32             = 5,  // 0x05 // deprecated
    Float           = 8,  // 0x08
    TextSelection   = 9,  // 0x09
    String          = 10, // 0x0A
    Folder          = 11, // 0x0B
    Info            = 12, // 0x0C
    Command         = 13, // 0x0D

    // This type will be sent if a parameter number out of 
    // the device parameter range will be requested. 
    // It will be also sent as the last parameter to 
    // let the host know the end of the parameter list 
    // on a Parameters settings list (read request).
    OutOfRange      = 0x7f,
};

}