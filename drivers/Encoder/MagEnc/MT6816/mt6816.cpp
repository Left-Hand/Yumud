#include "mt6816.hpp"


using namespace ymd::drivers;
using namespace ymd;

using Error = MT6816::Error;

template<typename T = void>
using IResult = MT6816::IResult<T>;


Result<void, Error> MT6816::init() {
    lap_position_ = -1; // not possible before init done;

    
    for(size_t i = 0; i < MAX_INIT_RETRY_TIMES; i++){
        this->update();
        if(this->get_lap_position().is_ok()){
            return Ok();
        }
    } // while reading before get correct position

    return Err(Error::CantSetup);
}

Result<uint16_t, hal::HalResult> MT6816::get_position_data(){
    uint16_t dataTx[2];
    uint16_t dataRx[2] = {0, 0};

    dataTx[0] = (0x80 | 0x03) << 8;
    dataTx[1] = (0x80 | 0x04) << 8;

    if(const auto err = spi_drv_.transfer_single(dataRx[1], dataTx[1]); err.is_err()) return Err(err);
    if(const auto err = spi_drv_.transfer_single(dataRx[0], dataTx[0]); err.is_err()) return Err(err);

    return Ok<uint16_t>(((dataRx[0] & 0x00FF) << 8) | (dataRx[1]));
}

IResult<> MT6816::update(){
    uint16_t raw_16 = get_position_data().unwrap();
    if(fast_mode_ == false){
        Semantic semantic;
        semantic = raw_16;
        last_sema_ = semantic;

        if(semantic.no_mag) return Err(Error::MagnetLost);

        uint8_t count = 0;

        raw_16 -= semantic.pc;
        while(raw_16){//Brian Kernighan algorithm
            raw_16 &= raw_16 - 1;
            ++count;
        }

        if(count % 2 == semantic.pc){
            lap_position_ = u16_to_uni(semantic.data_14bit << 2);
        }else{
            err_cnt_++;
            return Err(Error::WrongPc);
        }

    }else{
        lap_position_ = u16_to_uni(raw_16 & (0xfffc));
    }

    return Ok();
}