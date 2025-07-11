#include "MMC5983.hpp"
#include "core/debug/debug.hpp"

using namespace ymd;
using namespace ymd::drivers;

#define MMC5983_DEBUG_EN

#ifdef MMC5983_DEBUG_EN

#define CHECK_RES(x, ...) ({\
    const auto __res_check_res = (x);\
    ASSERT{__res_check_res.is_ok(), ##__VA_ARGS__};\
    __res_check_res;\
})\


#define CHECK_ERR(x, ...) ({\
    const auto && __err_check_err = (x);\
    ASSERT{false, #x, ##__VA_ARGS__};\
    __err_check_err;\
})\

#else
#define CHECK_RES(x, ...) (x)
#define CHECK_ERR(x, ...) (x)
#endif


using Error = ImuError;
template<typename T = void>
using IResult = Result<T, Error>;


IResult<> MMC5983::update(){
    return read_burst(data_packet_.address, data_packet_.as_bytes());
}


IResult<> MMC5983::init(const Config & cfg){
    if(const auto res = validate();
        res.is_err()) return res;

    clock::delay(1ms);


    if(const auto res = enable_mag_meas(EN);
        res.is_err()) return CHECK_RES(res);

    if(const auto res = enable_temp_meas(EN);
        res.is_err()) return CHECK_RES(res);

    if(const auto res = set_prd_magset(cfg.prd_set);
        res.is_err()) return CHECK_RES(res);

    if(const auto res = set_bandwidth(cfg.bandwidth);
        res.is_err()) return CHECK_RES(res);

    if(const auto res = set_odr(cfg.data_rate);
        res.is_err()) return CHECK_RES(res);

    return Ok();
}

IResult<> MMC5983::validate(){
    if(const auto res = read_reg(product_id_reg); 
        res.is_err()) return res;
    
    if(product_id_reg.product_id != product_id_reg.KEY)
        return CHECK_ERR(Err(Error::WrongWhoAmI));

    return Ok();
}

IResult<> MMC5983::reset(){
    auto reg = RegCopy(internal_control_1_reg);
    reg.sw_rst = 1;
    if(const auto res = write_reg(reg);
        res.is_err()) return res;
    reg.sw_rst = 0;
    clock::delay(1ms);
    if(const auto res = write_reg(reg);
        res.is_err()) return res;
    
    return Ok();
}

IResult<> MMC5983::set_odr(const Odr odr){
    auto reg = RegCopy(internal_control_2_reg);
    reg.data_rate = odr;
    reg.cmm_en = (odr != Odr::SingleShot);
    return write_reg(reg);
}

IResult<> MMC5983::set_bandwidth(const BandWidth bw){
    auto reg = RegCopy(internal_control_1_reg);
    reg.bw = bw;
    return write_reg(reg);
}

IResult<> MMC5983::enable_x(const Enable en){
    auto reg = RegCopy(internal_control_1_reg);
    reg.x_inhibit = (en == EN) ? false : true;
    return write_reg(reg);
}
IResult<> MMC5983::enable_yz(const Enable en){
    auto reg = RegCopy(internal_control_1_reg);
    reg.yz_inhibit = (en == EN) ? false : true;
    return write_reg(reg);
}

IResult<Vector3<q24>> MMC5983::read_mag(){
    static constexpr auto LSB_18BIT = 0.0000625_q24;

    const auto mag3i = data_packet_.to_vec3();
    // DEBUG_PRINTLN(mag3i);
    
    return Ok(Vector3<q24>{
        LSB_18BIT * mag3i.x,
        LSB_18BIT * mag3i.y,
        LSB_18BIT * mag3i.z
    });
}

IResult<q16> MMC5983::read_temp(){
    return Ok(data_packet_.to_temp());
}

IResult<bool> MMC5983::is_mag_meas_done(){
    auto & reg = status_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(bool(reg.meas_mag_done));
}


IResult<bool> MMC5983::is_temp_meas_done(){
    auto & reg = status_reg;
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
    return Ok(bool(reg.meas_temp_done));
}



IResult<> MMC5983::enable_mag_meas(const Enable en){
    auto reg = RegCopy(internal_control_0_reg);
    reg.tm_m = (en == EN)? true : false;
    return write_reg(reg);
}


IResult<> MMC5983::enable_temp_meas(const Enable en){
    auto reg = RegCopy(internal_control_0_reg);
    reg.tm_t = (en == EN)? true : false;
    return write_reg(reg);
}

IResult<> MMC5983::set_prd_magset(const PrdSet prdset){
    if(prdset != PrdSet::_1) 
        if(const auto res = enable_auto_mag_sr(EN);
            res.is_err()) return res;
    auto reg = RegCopy(internal_control_2_reg);
    reg.prd_set = prdset;
    return write_reg(reg);
}
IResult<> MMC5983::enable_magset(const Enable en){
    auto reg = RegCopy(internal_control_0_reg);
    reg.set = true;
    return write_reg(reg);
}

IResult<> MMC5983::enable_magreset(const Enable en){
    auto reg = RegCopy(internal_control_0_reg);
    reg.reset = true;
    return write_reg(reg);
}

template<typename Fn>
static auto do_setreset(MMC5983 & imu, Fn && fn) -> decltype(imu.read_mag()){
    if(const auto res = std::forward<Fn>(fn)(EN);
        res.is_err()) return Err(res.unwrap_err());
    for(auto m = clock::millis(); ;){
        if(const auto res = imu.is_mag_meas_done(); 
            res.is_err()) return Err(res.unwrap_err());
        else if(res.unwrap() == true)
            break;
        if(clock::millis() - m > 1000ms) return Err<ImuError>(ImuError(ImuError::Kind::MagCantSetup));
    }
    if(const auto res = imu.update();
        res.is_err()) return Err(res.unwrap_err());

    if(const auto res = std::forward<Fn>(fn)(DISEN);
        res.is_err()) return Err(res.unwrap_err());
    return imu.read_mag();
};


MMC5983::IResult<Vector3<q24>> MMC5983::do_magset(){
    return do_setreset(*this, [this](Enable en){return enable_magset(en);});
}

MMC5983::IResult<Vector3<q24>> MMC5983::do_magreset(){
    return do_setreset(*this, [this](Enable en){return enable_magreset(en);});
}

IResult<> MMC5983::enable_auto_mag_sr(const Enable en){
    auto reg = RegCopy(internal_control_0_reg);
    reg.auto_sr_en = en == EN ? true : false;
    return write_reg(reg);
}