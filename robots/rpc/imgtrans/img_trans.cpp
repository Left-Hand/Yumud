#include "img_trans.hpp"

scexpr uint32_t hash_djb2_buffer(const uint8_t *p_buff, int p_len, uint32_t p_prev = 5381) {
	uint32_t hash = p_prev;

	for (int i = 0; i < p_len; i++) {
		hash = ((hash << 5) + hash) + p_buff[i]; /* hash * 33 + c */
	}

	return hash;
}

void Transmitter::sendBlockData(ImagePieceUnit & unit, const uint8_t * data_from, const size_t len){

    unit.header = header;
    unit.hash = hash_djb2_buffer(data_from, len);
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
