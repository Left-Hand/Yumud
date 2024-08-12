#pragma once

#include "src/testbench/tb.h"
#include "types/image/image.hpp"

#pragma pack(push, 1)
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
#pragma pack(pop)



class Transmitter{
protected:
    // static constexpr size_t str_tx_buf_size = 512;
    // static constexpr size_t str_rx_buf_size = 512;
    // static constexpr size_t img_tx_buf_size = 1024;

public:

    OutputStream & instance;
    bool enabled = false;
    Uart & logger = DEBUGGER;

    // RingBuf<str_tx_buf_size> str_tx_buf;
    // RingBuf<str_rx_buf_size> str_rx_buf;
    // RingBuf<img_tx_buf_size> img_buf;

    // void write(const char data) override{
    //     str_rx_buf.addData(data);
    // }

    // void read(char & data) override{
    //     data = str_rx_buf.getData();
    // }

    // using InputStream::read;

    // size_t available() const override{
    //     return str_rx_buf.available();
    // }

    // size_t pending() const override{
    //     return str_tx_buf.available();
    // }

protected:
    static constexpr uint16_t header = 0x54A8;
    static constexpr size_t mtu = 320;
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
