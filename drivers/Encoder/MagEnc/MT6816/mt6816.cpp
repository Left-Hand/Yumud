#include "mt6816.hpp"


using namespace ymd::drivers;
using namespace ymd;

using Error = MT6816::Error;

template<typename T = void>
using IResult = MT6816::IResult<T>;

// #define MT6816_DEBUG_EN 1
#define MT6816_DEBUG_EN 0

#if MT6816_DEBUG_EN
#define CHECK_RES(x, ...) ({\
    const auto __res_check_res = (x);\
    ASSERT{__res_check_res.is_ok(), __LINE__, ##__VA_ARGS__};\
    __res_check_res;\
})\


#define CHECK_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    ASSERT{false, #x, __LINE__, ##__VA_ARGS__};\
    __err_check_err;\
})\

#else

#define CHECK_RES(x, ...) (x)
#define CHECK_ERR(x, ...) (x)
#endif


IResult<> MT6816::init(const Config & cfg) {
    static constexpr size_t MAX_INIT_RETRY_TIMES = 32;
    static constexpr size_t PRE_WAKEUP_TIMES = 30;

    if(const auto res = reconf(cfg); 
        res.is_err()) return res;

    [[maybe_unused]] auto pre_wakeup = [&]{
        for(size_t i = 0; i < PRE_WAKEUP_TIMES; i++){
            if(const auto res = this->get_position_data();
                res.is_ok()){
                //pass
            }else{
                //pass
            }
        }
    };


    //上电初始时会读出全零数据 需要跳过这个时期的脏数据
    pre_wakeup();
    
    for(size_t i = 0;; i++){
        if(const auto res = this->update();
            res.is_err()) return res;

        if(const auto res = this->read_lap_angle();
            res.is_ok()) return Ok();
        else if(i == MAX_INIT_RETRY_TIMES)
            return CHECK_ERR(Err(res.unwrap_err()));
        clock::delay(1ms);
    } // while reading before get correct position

    return Ok();
}

IResult<> MT6816::reconf(const Config & cfg){
    fast_mode_ = cfg.fast_mode_en == EN;
    return Ok();
}

IResult<uint16_t> MT6816::get_position_data(){
    const uint16_t tx[2] = {
        uint16_t((0x80 | 0x04) << 8),
        uint16_t((0x80 | 0x03) << 8),
    };

    uint16_t rx[2] = {0, 0};

    if(const auto res = spi_drv_.transceive_burst(
            std::span(rx), std::span(tx));
        res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));

    return Ok<uint16_t>(((rx[1] & 0x00FF) << 8) | (rx[0] & 0x00FF));
}

IResult<> MT6816::update(){
    const uint16_t raw_16 = ({
        const auto res = get_position_data();
        if(res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
        res.unwrap();
    });

    Semantic semantic = raw_16;

    if(fast_mode_ == false){
        last_sema_ = semantic;

        if(semantic.no_mag)
            return CHECK_ERR(Err(Error::MagnetLost));

        uint8_t count = 0;

        uint16_t raw_16_copy = raw_16;
        raw_16_copy -= semantic.pc;
        while(raw_16_copy){//Brian Kernighan algorithm
            raw_16_copy &= raw_16_copy - 1;
            ++count;
        }

        if(count % 2 == semantic.pc){
            lap_position_ = semantic.to_position();
        }else{
            err_cnt_++;
            return CHECK_ERR(Err(Error::InvalidCrc), std::bitset<16>(raw_16));
        }

    }else{
        lap_position_ = semantic.to_position();
    }

    return Ok();
}