#pragma once

#include "sys/core/system.hpp"
#include "sys/debug/debug_inc.h"

#include "types/image/image.hpp"

struct TransType{
    uint8_t index:4;
    uint8_t type:4;

    enum {
        GS_0 = 0x00,
        GS_1,
        GS_2,
        GS_3,
        BN_0 = 0x10,
        BN_1,
        BN_2,
        BN_3,
        RGB = 0x20,
        STR = 0x30
    };
}__packed;


struct PieceHeader{
    uint16_t header;
    TransType trans_type;
}__packed;

struct ImagePieceUnit:public PieceHeader{
    uint32_t hash;
    uint8_t time_stamp;
    uint8_t size_x;
    uint8_t size_y;
    uint16_t data_index;
}__packed;


struct OutputStream;
struct Uart;


class Transmitter{
public:

    OutputStream & instance;
    bool enabled = false;
    Uart & logger = DEBUGGER;

protected:
    static constexpr uint16_t header = 0x54A8;
    static constexpr size_t mtu = 320;
    uint8_t time_stamp = 0;
    void transmit(const uint8_t * img_buf, const Vector2i & img_size, const uint8_t index);

    void sendBlockData(ImagePieceUnit & unit, const uint8_t * data_from, const size_t len);
public:
    Transmitter(OutputStream & _instance):instance(_instance){;}

    void transmit(const Image<monochrome auto> & img, const uint8_t index){
        transmit((const uint8_t *)img.get_data(),img.get_size(), index); 
    }

    void enable(const bool en){
        enabled = en;
    }
};
