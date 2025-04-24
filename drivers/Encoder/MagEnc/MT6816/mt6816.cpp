#include "mt6816.hpp"


using namespace ymd::drivers;
using namespace ymd;



void MT6816::init() {
    last_semantic_ = 0;
    lap_position_ = -1; // not possible before init done;
    while(get_lap_position().is_none()){
        this->update();
    } // while reading before get correct position
}

uint16_t MT6816::get_position_data(){
    uint16_t dataTx[2];
    uint16_t dataRx[2] = {0, 0};

    dataTx[0] = (0x80 | 0x03) << 8;
    dataTx[1] = (0x80 | 0x04) << 8;

    if(const auto err = spi_drv_.transfer_single(dataRx[1], dataTx[1]); err.ok()) return 0;
    if(const auto err = spi_drv_.transfer_single(dataRx[0], dataTx[0]); err.ok()) return 0;

    return((dataRx[0] & 0x00FF) << 8) | (dataRx[1]);
}

void MT6816::update() {
    uint16_t raw = get_position_data();
    if(fast_mode_ == false){
        last_semantic_ = raw;

        uint8_t count = 0;

        raw -= last_semantic_.pc;
        while(raw){//Brian Kernighan algorithm
            raw &= raw - 1;
            ++count;
        }

        if(count % 2 == last_semantic_.pc){
            lap_position_ = u16_to_uni(last_semantic_.data_14bit << 2);
        }else{
            err_cnt_++;
        }
    }else{
        lap_position_ = u16_to_uni(raw);
    }
}
