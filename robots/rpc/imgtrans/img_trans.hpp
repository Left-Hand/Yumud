#pragma once


#include "core/stream/ostream.hpp"
#include "types/image/image.hpp"

namespace ymd{
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
};


struct PieceHeader{
    uint16_t header;
    TransType trans_type;
};

struct ImagePieceUnit:public PieceHeader{
    uint32_t hash;
    uint8_t time_stamp;
    uint8_t size_x;
    uint8_t size_y;
    uint16_t data_index;
};




class Transmitter{
protected:
    // scexpr size_t str_tx_buf_size = 512;
    // scexpr size_t str_rx_buf_size = 512;
    // scexpr size_t img_tx_buf_size = 1024;
    using Vector2 = ImageBasics::Vector2;
    using Vector2i = ImageBasics::Vector2i;
public:

    OutputStream & instance;
    bool enabled = false;
    // OutputStream & logger;

protected:
    scexpr uint16_t header = 0x54A8;
    scexpr size_t mtu = 320;
    uint8_t time_stamp = 0;
    void transmit(const uint8_t * img_buf, const Vector2i & img_size, const uint8_t index);

    void sendBlockData(ImagePieceUnit & unit, const uint8_t * data_from, const size_t len);
public:
    Transmitter(OutputStream & _instance):instance(_instance){;}


    template<typename T>
    requires std::is_same_v<T, Binary> || std::is_same_v<T, Grayscale>
    void transmit(const Image<T> & img, const uint8_t index){
        transmit((const uint8_t *)img.get_data(),img.get_size(), index); 
    }

    void enable(const bool en){
        enabled = en;
    }
};

}
