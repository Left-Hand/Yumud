#include "LIS3DH.hpp"

// #define LIS3DH_DEBUG_EN
#define LIS3DH_CHEAT_EN

#ifdef LIS3DH_DEBUG_EN
#define LIS3DH_TODO(...) TODO()
#define LIS3DH_DEBUG(...) DEBUG_PRINTS(__VA_ARGS__);
#define LIS3DH_PANIC(...) PANIC{__VA_ARGS__}
#define LIS3DH_ASSERT(cond, ...) ASSERT{cond, ##__VA_ARGS__}
#else
#define LIS3DH_DEBUG(...)
#define LIS3DH_TODO(...) PANIC_NSRC()
#define LIS3DH_PANIC(...)  PANIC_NSRC()
#define LIS3DH_ASSERT(cond, ...) ASSERT_NSRC(cond)
#endif

using namespace ymd;
using namespace ymd::drivers;

using Error = LIS3DH::Error;
template<typename T = void>
using IResult = Result<T, Error>;
Result<void, Error> LIS3DH::init(){
	// LIS_Write_Byte(CTRL_REG1,0x3F);			//设置低功耗模式 xyz轴使能 采样频率25HZ
	// LIS_Write_Byte(CTRL_REG2,0x09);			//高通滤波正常模式，数据从内部滤波器到输出寄存器 高通滤波器使能到中断1
	// LIS_Write_Byte(CTRL_REG3,0x40);			//使能AOI中断1
	// LIS_Write_Byte(CTRL_REG4,0x80);			//分辨率为+-2g  则单位为 4000/(2^8)=16mg       仅在LSB和MSB被读时才更新数据
	// LIS_Write_Byte(CTRL_REG5,0x08);			//寄存器锁存中断，只有读INT1_SRC可以恢复
	// LIS_Write_Byte(INT1_THS,0x10);			//设置阈值 16*16=256 mg
	// LIS_Write_Byte(INT1_DURATION,0x00);		//中断持续时间设置，此处设为0，因为不读INT1_SRC中断就一直锁存
	// LIS_Read_Byte(REFERENCE);				//读虚寄存器,强制将当前加速度值加载到高通滤波器值进行比较
 
	// //INT1_CFG使能中断  中断模式为 “OR”组合 只使能xyz轴高事件
	// //LIS_Write_Byte(INT1_CFG,0x2A);
	// //INT1_CFG使能中断  中断模式6-direction position  使能xyz轴高低事件
	// LIS_Write_Byte(INT1_CFG,0xFF);
	// LIS_Read_Byte(INT1_SRC);//清除中断位
    return Ok();
}


Result<void, Error> LIS3DH::update(){
    return Ok();
}


Result<void, Error> LIS3DH::clear_flag(){
    // return read_reg(regs_.int1_src_reg)
    return Ok();
}


Result<void, Error> LIS3DH::validate(){
    auto & reg = regs_.whoami_reg;

    if(const auto res = verify_phy();
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = reset();
        res.is_err()) return Err(res.unwrap_err());
    if(const auto res = read_reg(reg);
        res.is_err()) return Err(res.unwrap_err());
        
    if(reg.to_bits() != reg.key)
        return Err(Error::InvalidChipId);
    
    return Ok();
}


Result<void, Error> LIS3DH::reset(){
    auto reg = RegCopy(regs_.ctrl1_reg);
    reg.sw_reset = 1;
    auto err = write_reg(reg);
    reg.sw_reset = 0;
    return err;
}



IResult<Vec3<iq24>> LIS3DH::read_acc(){
    return Ok(Vec3<iq24>::ZERO);

}

