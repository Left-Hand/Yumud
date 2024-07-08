#pragma once

#include "src/testbench/tb.h"

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



class Transmitter:public IOStream{
protected:
    static constexpr size_t str_tx_buf_size = 512;
    static constexpr size_t str_rx_buf_size = 512;
    static constexpr size_t img_tx_buf_size = 1024;

public:

    IOStream & instance;
    Uart & logger = uart2;

    RingBuf<str_tx_buf_size> str_tx_buf;
    RingBuf<str_rx_buf_size> str_rx_buf;
    RingBuf<img_tx_buf_size> img_buf;

    void write(const char data) override{
        str_rx_buf.addData(data);
    }

    void read(char & data) override{
        data = str_rx_buf.getData();
    }

    using InputStream::read;

    size_t available() const override{
        return str_rx_buf.available();
    }

    size_t pending() const override{
        return str_tx_buf.available();
    }

protected:
    static constexpr uint16_t header = 0x54A8;
    uint8_t time_stamp;
public:
    Transmitter(IOStream & _instance):instance(_instance){;}
    void sendBlockData(ImagePieceUnit & unit, const uint8_t * data_from, const size_t len);

    std::vector<uint8_t> compress_png(const Image<Grayscale, Grayscale> & img);

    void transmit(const Image<Grayscale, Grayscale> & img, const uint8_t index);
};
