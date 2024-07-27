#pragma once

#include "../../sys/debug/debug_inc.h"

#include <array>
#include <cstdint>
#include <algorithm>
#include <tuple>

class ApriltagDecoder{

};

class Apriltag16H5Decoder:public ApriltagDecoder{
public:
    static constexpr std::array<uint16_t, 30> codes= {
        0x231b,
        0x2ea5,
        0x346a,
        0x45b9,
        0x79a6,
        0x7f6b,
        0xb358,
        0xe745,
        0xfe59,
        0x156d,
        0x380b,
        0xf0ab,
        0x0d84,
        0x4736,
        0x8c72,
        0xaf10,
        0x093c,
        0x93b4,
        0xa503,
        0x468f,
        0xe137,
        0x5795,
        0xdf42,
        0x1c1d,
        0xe9dc,
        0x73ad,
        0xad5f,
        0xd530,
        0x07ca,
        0xaf2e
    };

    static std::array<uint16_t, 120> all_codes;
protected:
    uint16_t last_code = 0;
    uint16_t last_angle = 0;
    int last_index = 0;


    static uint16_t reverse16Bits(uint16_t num) {
        uint16_t reversed = 0;

        reversed |= (num & (1 << 12)) ? (1 << 15) : 0;
        reversed |= (num & (1 << 8)) ? (1 << 14) : 0;
        reversed |= (num & (1 << 4)) ? (1 << 13) : 0;
        reversed |= (num & (1 << 0)) ? (1 << 12) : 0;

        reversed |= (num & (1 << 13)) ? (1 << 11) : 0;
        reversed |= (num & (1 << 9)) ? (1 << 10) : 0;
        reversed |= (num & (1 << 5)) ? (1 << 9) : 0;
        reversed |= (num & (1 << 1)) ? (1 << 8) : 0;

        reversed |= (num & (1 << 14)) ? (1 << 7) : 0;
        reversed |= (num & (1 << 10)) ? (1 << 6) : 0;
        reversed |= (num & (1 << 6)) ? (1 << 5) : 0;
        reversed |= (num & (1 << 2)) ? (1 << 4) : 0;

        reversed |= (num & (1 << 15)) ? (1 << 3) : 0;
        reversed |= (num & (1 << 11)) ? (1 << 2) : 0;
        reversed |= (num & (1 << 7)) ? (1 << 1) : 0;
        reversed |= (num & (1 << 3)) ? (1 << 0) : 0;
        return reversed;
    }

    static uint16_t reverse4Bits(uint16_t num) {
        uint16_t reversed = 0;
        for (int i = 0; i < 16; ++i) {
            reversed <<= 1; // Shift left by one bit
            reversed |= (num & 1); // Add the least significant bit of num to reversed
            num >>= 1; // Shift num right by one bit to process the next bit
        }
        return reversed;
    }

    static void generate_all_codes(){
        for(size_t i = 0; i < codes.size(); i++){

            //FIXME
            auto get_rcr_code = [](const uint16_t code, const uint8_t times) -> uint16_t{
                // return (code << (4 * times) | (code >> (16 - 4 * times)));
                switch(times){
                    case 0:return code;
                    case 2:
                        return reverse4Bits(code);
                    case 3:{
                        //bit reserve
                        return reverse16Bits(code);
                    }
                    case 1:
                        return reverse16Bits(reverse4Bits(code));
                }
                return 0;
            };

            for(uint8_t times = 0; times < 4; times++){
                all_codes[times * codes.size() + i] = get_rcr_code(codes[i], times);
            }
        }
    }
    static std::tuple<uint16_t, uint16_t> find_code(const uint16_t new_code){
        uint16_t new_index = 0;
        uint16_t new_angle = 0;

        auto it = std::find(all_codes.begin(), all_codes.end(), new_code);
        if(it != all_codes.end()){
            const auto id = (std::distance(all_codes.begin(), it) + 1);
            new_index = id % codes.size();
            new_angle = id / codes.size();
        }
        return {new_index, new_angle};
    }
public:
    Apriltag16H5Decoder(){generate_all_codes();}
    uint16_t update(const uint16_t new_code){
        auto [new_index, new_angle] = find_code(new_code);

        if(new_index != 0){
            last_code = new_code;
            last_index = new_index;
            last_angle = new_angle;
        }

        return last_index;
    }

    uint16_t index() const{
        return last_index;
    }

    uint16_t code() const{
        return last_code;
    }

    uint16_t angle() const{
        return last_angle;
    }

    bool is_valid() const {
        return last_code != 0;
    }
    
    operator bool() const{
        return is_valid();
    }
};