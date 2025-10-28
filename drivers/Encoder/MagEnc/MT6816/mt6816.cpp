#include "mt6816.hpp"


using namespace ymd::drivers;
using namespace ymd;


using Self = MT6816;
using Error = Self::Error;

template<typename T = void>
using IResult = Self::IResult<T>;

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


IResult<> Self::init(const Config & cfg) {
    static constexpr size_t MAX_INIT_RETRY_TIMES = 32;
    static constexpr size_t MAX_PRE_WAKEUP_TIMES = 30;

    if(const auto res = reconf(cfg); 
        res.is_err()) return res;

    //上电初始时会读出全零数据 需要跳过这个时期的脏数据
    for(size_t i = 0; i < MAX_PRE_WAKEUP_TIMES; i++){
        if(const auto res = this->get_packet();
            res.is_err()){
            //pass
        }else{
            //pass
        }
    }
    
    for(size_t i = 0;; i++){
        if(const auto res = this->update();
            res.is_err()) return res;

        if(const auto res = this->read_lap_angle();
            res.is_err()){
            if(i == MAX_INIT_RETRY_TIMES)
                return CHECK_ERR(Err(res.unwrap_err()));
            clock::delay(1ms);
        }else{
            return Ok();
        }

    } // while reading before get correct position

    return Ok();
}

IResult<Self::Packet> Self::get_packet(){
    const uint16_t tx[2] = {
        uint16_t((0x80 | 0x04) << 8),
        uint16_t((0x80 | 0x03) << 8),
    };

    uint16_t rx[2] = {0, 0};

    if(const auto res = spi_drv_.transceive_burst(
            std::span(rx), std::span(tx));
        res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));

    const uint16_t packet_underlying = 
        (static_cast<uint16_t>(rx[1] & 0x00FF) << 8) | 
        static_cast<uint16_t>(rx[0] & 0x00FF);
    return Ok(std::bit_cast<Packet>(packet_underlying));
}

IResult<> Self::update(){
    last_packet_ = ({
        const auto res = get_packet();
        if(res.is_err()) return CHECK_ERR(Err(res.unwrap_err()));
        res.unwrap();
    });

    return Ok();
}