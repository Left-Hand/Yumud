#include "img_trans.hpp"
#include "algo/hash_func.hpp"

void Transmitter::sendBlockData(ImagePieceUnit & unit, const uint8_t * data_from, const size_t len){

    unit.header = header;
    unit.hash = hash_impl(data_from, len);
    unit.time_stamp = time_stamp;

    instance.write((const char *)&unit, sizeof(unit));
    instance.write((const char *)(data_from), len);
}

void Transmitter::transmit(const uint8_t *buf, const Vector2i &img_size, const uint8_t index){
    if(!enabled) return;
    size_t len = img_size.x * img_size.y;
    uint8_t block_total = len / mtu + bool(len % mtu);
    uint8_t block_number = 0;

    while(true){

        uint16_t block_start = block_number * mtu;
        uint16_t block_end = block_start + std::min(len - block_start, mtu);

        ImagePieceUnit unit;
        unit.size_x = img_size.x;
        unit.size_y = img_size.y;
        unit.trans_type = TransType(index);
        unit.data_index = block_start;

        sendBlockData(unit, (const uint8_t *)buf + block_start, block_end - block_start);

        block_number++;

        if(block_number >= block_total){
            break;
        }

    }

    time_stamp++;
}
