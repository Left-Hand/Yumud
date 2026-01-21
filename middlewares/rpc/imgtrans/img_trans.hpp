#pragma once


#include "core/stream/ostream.hpp"
#include "primitive/image/image.hpp"

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

struct Inner {
    int x;
    double y;
};


struct ImagePieceUnit{
    using Self = ImagePieceUnit;
    uint16_t header;
    TransType trans_type;
    uint32_t hash;
    uint8_t time_stamp;
    uint8_t size_x;
    uint8_t size_y;
    uint16_t data_index;

    [[nodiscard]] std::span<const uint8_t> as_bytes() const {
        return std::span<const uint8_t>(reinterpret_cast<const uint8_t *>(this), sizeof(Self));
    }
};




class ImageTransmitter{
protected:
    // static constexpr size_t str_tx_buf_size = 512;
    // static constexpr size_t str_rx_buf_size = 512;
    // static constexpr size_t img_tx_buf_size = 1024;
public:

    OutputStream & instance;
    bool enabled = false;
    // OutputStream & logger;

protected:
    static constexpr uint16_t header = 0x54A8;
    static constexpr size_t mtu = 320;
    uint8_t time_stamp = 0;
    void transmit(const uint8_t * img_buf, const Vec2i & img_size, const uint8_t index);

    void send_block_data(ImagePieceUnit & unit, const uint8_t * data_from, const size_t len);
public:
    ImageTransmitter(OutputStream & _instance):instance(_instance){;}


    template<typename T>
    requires std::is_same_v<T, Binary> || std::is_same_v<T, Gray>
    void transmit(const Image<T> & img, const uint8_t index){
        transmit((const uint8_t *)img.get_data(),img.get_size(), index);
    }

    void enable(const Enable en){
        enabled = en == EN;
    }
};

}
