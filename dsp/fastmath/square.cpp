#include "square.hpp"
#include "array"

uint16_t fast_square8(const uint8_t x){
    static std::array<uint16_t, 256> q_map{[]{
        std::array<uint16_t, 256> temp = {};
        for(int i = 0; i < 256; ++i) {
            temp[i] = (i * i);
        }
        return temp;
    }()};

    return q_map[x];
}
