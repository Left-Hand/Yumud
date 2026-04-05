#pragma once

#include "core/io/regs.hpp"
#include "drivers/encoder/encoder.hpp"

#include "hal/conn/spi/spidrv.hpp"

//参考文档：https://www.cnblogs.com/c17VV-blogs/p/18864288


namespace ymd::drivers{

struct TLE5012_Prelude{
using Error = EncoderError;
template<typename T = void>
using IResult = Result<T, Error>;
enum class [[nodiscard]] RegAddr:uint8_t{
	STAT    =  0,     
	ACSTAT  =  1,
	AVAL    =  2,
	ASPD    =  3,
	AREV    =  4,
	FSYNC   =  5,
	MOD_1   =  6,
	SIL     =  7,
	MOD_2   =  8,
	MOD_3   =  9,
	OFFX    = 10,     
	OFFY    = 11,     
	SYNCH   = 12,
	IFAB    = 13,
	MOD_4   = 14,
	TCO_Y   = 15,
	ADC_X   = 16,
	ADC_Y   = 17,  
	D_MAG   = 18,
	T_RAW   = 19,
	IIF_CNT = 20,
	T25O    = 21, 
};


static constexpr uint32_t TLE5012_CRC_POLYNOMIAL              = 0x1D;
static constexpr uint32_t TLE5012_CRC_SEED                    = 0xFF;


[[nodiscard]] static constexpr uint8_t crc8(std::span<const uint8_t> bytes) {
	uint32_t crc;

	crc = TLE5012_CRC_SEED;
	for (size_t i = 0; i < bytes.size(); i++) {
		crc ^= bytes[i];
		for (size_t bit = 0; bit < 8; bit++) {
			if ((crc & 0x80) != 0) {
				crc <<= 1;
				crc ^= TLE5012_CRC_POLYNOMIAL;
			} else {
				crc <<= 1;
			}
		}
	}

	return ((~crc) & TLE5012_CRC_SEED);
}



struct [[nodiscard]] SafetyWord final{
    uint16_t crc:8;
    uint16_t sensor_number:4;
    uint16_t invalid_angle_value:1;
    uint16_t interface_access_error:1;
    uint16_t system_error:1;
    uint16_t watchdog_overflow:1;

    constexpr Result<void, void> check(const uint16_t command, const uint16_t readreg) const {
        auto & self = *this;
		uint8_t temp[4] = {
            [0] = (uint8_t) (command >> 8),
            [1] = (uint8_t) (command),
    
            [2] = (uint8_t) (readreg >> 8),
            [2 + 1] = (uint8_t) (readreg),
        };

		const uint8_t crc_actual = uint8_t(self.crc);
		const uint8_t crc_expected = crc8(std::span(temp));

		if(crc_actual != crc_expected) return Err();
        return Ok();
    }
};
};

struct TLE5012_Regset final: public TLE5012_Prelude{


// 0x00 STAT - Status Register
struct [[nodiscard]] R16_STAT : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::STAT;
    static constexpr uint16_t RESET_VALUE = 0x8001;
    
    uint16_t s_rst     : 1;  // Bit0  复位状态
    uint16_t s_wd      : 1;  // Bit1  看门狗状态
    uint16_t s_vr      : 1;  // Bit2  稳压器状态
    uint16_t s_fuse    : 1;  // Bit3  熔丝CRC状态
    uint16_t s_dspu    : 1;  // Bit4  DSPU自测试
    uint16_t s_ov      : 1;  // Bit5  DSPU溢出
    uint16_t s_xyol    : 1;  // Bit6  X/Y超限
    uint16_t s_magol   : 1;  // Bit7  磁场幅值超限
    uint16_t res8      : 1;  // Bit8  保留
    uint16_t s_adct    : 1;  // Bit9  ADC测试状态
    uint16_t s_rom     : 1;  // Bit10 ROM校验状态
    uint16_t no_gmr_xy : 1;  // Bit11 GMR X/Y无效
    uint16_t no_gmr_a  : 1;  // Bit12 GMR角度无效
    uint16_t s_nr      : 2;  // Bit14-13 从机编号
    uint16_t rd_st     : 1;  // Bit15 读状态标志
};

VALIDATE_R16(R16_STAT)

// 0x01 ACSTAT - Activation Status Register
struct [[nodiscard]] R16_ACSTAT : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::ACSTAT;
    static constexpr uint16_t RESET_VALUE = 0x5EFE;
    
    uint16_t as_rst    : 1;  // Bit0  硬件复位激活
    uint16_t as_wd     : 1;  // Bit1  看门狗使能
    uint16_t as_vr     : 1;  // Bit2  稳压器检测使能
    uint16_t as_fuse   : 1;  // Bit3  熔丝CRC使能
    uint16_t as_dspu   : 1;  // Bit4  DSPU自测试使能
    uint16_t as_ov     : 1;  // Bit5  溢出检测使能
    uint16_t as_vec_xy : 1;  // Bit6  X/Y超限检测使能
    uint16_t as_vec_mag: 1;  // Bit7  幅值检测使能
    uint16_t res8      : 1;  // Bit8  保留
    uint16_t as_adct   : 1;  // Bit9  ADC测试使能
    uint16_t res15_10  : 6;  // Bit15-10 保留
};

VALIDATE_R16(R16_ACSTAT)

// 0x02 AVAL - Angle Value Register
struct [[nodiscard]] R16_AVAL : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::AVAL;
    static constexpr uint16_t RESET_VALUE = 0x8000;
    
    uint16_t ang_val   : 15; // Bit14-0 角度值
    uint16_t rd_av     : 1;  // Bit15 角度值更新标志
};

VALIDATE_R16(R16_AVAL)

// 0x03 ASPD - Angular Speed Register
struct [[nodiscard]] R16_ASPD : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::ASPD;
    static constexpr uint16_t RESET_VALUE = 0x8000;
    
    uint16_t ang_spd   : 15; // Bit14-0 角速度
    uint16_t rd_as     : 1;  // Bit15 角速度更新标志
};

VALIDATE_R16(R16_ASPD)

// 0x04 AREV - Angle Revolution Counter
struct [[nodiscard]] R16_AREV : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::AREV;
    static constexpr uint16_t RESET_VALUE = 0x8000;
    
    uint16_t revol     : 9;  // Bit8-0 旋转次数
    uint16_t fcnt      : 6;  // Bit14-9 帧计数器
    uint16_t rd_rev    : 1;  // Bit15 旋转计数更新标志
};

VALIDATE_R16(R16_AREV)

// 0x05 FSYNC - Frame Sync Register
struct [[nodiscard]] R16_FSYNC : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::FSYNC;
    static constexpr uint16_t RESET_VALUE = 0x0000;
    
    uint16_t res8_0    : 9;  // Bit8-0 保留
    uint16_t fsync     : 7;  // Bit15-9 帧同步值
};

VALIDATE_R16(R16_FSYNC)

// 0x06 MOD_1 - Mode Register 1
struct [[nodiscard]] R16_MOD1 : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::MOD_1;
    static constexpr uint16_t RESET_VALUE = 0x8001;
    
    uint16_t iif_mod   : 2;  // Bit1-0 增量接口模式
    uint16_t dspu_hold : 1;  // Bit2 DSPU保持
    uint16_t ssc_od    : 1;  // Bit3 SSC开漏/推挽
    uint16_t clk_sel   : 1;  // Bit4 时钟源选择
    uint16_t res13_5   : 9;  // Bit13-5 保留
    uint16_t fir_md    : 2;  // Bit15-14 滤波器配置
};

VALIDATE_R16(R16_MOD1)

// 0x07 SIL - Safety Test Register
struct [[nodiscard]] R16_SIL : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::SIL;
    static constexpr uint16_t RESET_VALUE = 0x0000;
    
    uint16_t adctv_x   : 3;  // Bit2-0 X通道测试向量
    uint16_t adctv_y   : 3;  // Bit5-3 Y通道测试向量
    uint16_t adctv_en  : 1;  // Bit6 ADC测试使能
    uint16_t res9_7    : 3;  // Bit9-7 保留
    uint16_t fuse_rel  : 1;  // Bit10 熔丝重载
    uint16_t res13_11  : 3;  // Bit13-11 保留
    uint16_t filt_inv  : 1;  // Bit14 滤波器反转
    uint16_t filt_par  : 1;  // Bit15 滤波器并行
};

VALIDATE_R16(R16_SIL)

// 0x08 MOD_2 - Mode Register 2
struct [[nodiscard]] R16_MOD2 : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::MOD_2;
    static constexpr uint16_t RESET_VALUE = 0x0800;
    
    uint16_t autocal   : 2;  // Bit1-0 自校准模式
    uint16_t predict   : 1;  // Bit2 预测使能
    uint16_t ang_dir   : 1;  // Bit3 角度方向
    uint16_t ang_range : 11; // Bit14-4 角度范围
    uint16_t res15     : 1;  // Bit15 保留
};

VALIDATE_R16(R16_MOD2)

// 0x09 MOD_3 - Mode Register 3
struct [[nodiscard]] R16_MOD3 : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::MOD_3;
    static constexpr uint16_t RESET_VALUE = 0x0000;
    
    uint16_t pad_drv   : 2;  // Bit1-0 焊盘驱动
    uint16_t res2      : 1;  // Bit2 保留
    uint16_t spikef    : 1;  // Bit3 尖峰滤波器
    uint16_t ang_base  : 12; // Bit15-4 角度基准
};

VALIDATE_R16(R16_MOD3)

// 0x0A OFFX - X Offset Register
struct [[nodiscard]] R16_OFFX : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::OFFX;
    static constexpr uint16_t RESET_VALUE = 0x0000;
    
    uint16_t res3_0    : 4;  // Bit3-0 保留
    uint16_t x_offset  : 12; // Bit15-4 X偏移
};

VALIDATE_R16(R16_OFFX)

// 0x0B OFFY - Y Offset Register
struct [[nodiscard]] R16_OFFY : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::OFFY;
    static constexpr uint16_t RESET_VALUE = 0x0000;
    
    uint16_t res3_0    : 4;  // Bit3-0 保留
    uint16_t y_offset  : 12; // Bit15-4 Y偏移
};

VALIDATE_R16(R16_OFFY)

// 0x0C SYNCH - Magnitude Synchronization
struct [[nodiscard]] R16_SYNCH : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::SYNCH;
    static constexpr uint16_t RESET_VALUE = 0x0000;
    
    uint16_t res3_0    : 4;  // Bit3-0 保留
    uint16_t synch     : 12; // Bit15-4 幅值同步
};

VALIDATE_R16(R16_SYNCH)

// 0x0D IFAB - IFAB Configuration Register
struct [[nodiscard]] R16_IFAB : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::IFAB;
    static constexpr uint16_t RESET_VALUE = 0x0003;
    
    uint16_t ifab_hyst : 2;  // Bit1-0 迟滞配置
    uint16_t ifab_od   : 1;  // Bit2 输出类型
    uint16_t res3      : 1;  // Bit3 保留
    uint16_t ortho     : 12; // Bit15-4 正交校正
};

VALIDATE_R16(R16_IFAB)

// 0x0E MOD_4 - Mode Register 4
struct [[nodiscard]] R16_MOD4 : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::MOD_4;
    static constexpr uint16_t RESET_VALUE = 0x0000;
    
    uint16_t if_md     : 3;  // Bit2-0 接口模式
    uint16_t ifab_res  : 2;  // Bit4-3 分辨率
    uint16_t res8_5    : 4;  // Bit8-5 保留
    uint16_t tco_x_t   : 7;  // Bit15-9 X温度系数
};

// 0x0F TCO_Y - Y Temperature Coefficient + CRC
struct [[nodiscard]] R16_TCOY : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::TCO_Y;
    static constexpr uint16_t RESET_VALUE = 0x0000;
    
    uint16_t crc_par   : 8;  // Bit7-0 参数CRC
    uint16_t res8      : 1;  // Bit8 保留
    uint16_t tco_y_t   : 7;  // Bit15-9 Y温度系数
};

// 0x10 ADC_X - X ADC Raw Value
struct [[nodiscard]] R16_ADCX : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::ADC_X;
    static constexpr uint16_t RESET_VALUE = 0x0000;
    
    uint16_t adc_x     : 16; // Bit15-0 X原始ADC值
};

// 0x11 ADC_Y - Y ADC Raw Value
struct [[nodiscard]] R16_ADCY : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::ADC_Y;
    static constexpr uint16_t RESET_VALUE = 0x0000;
    
    uint16_t adc_y     : 16; // Bit15-0 Y原始ADC值
};

// 0x20 IIF_CNT - Incremental Interface Counter
struct [[nodiscard]] R16_IIFCNT : public Reg16<> {
    static constexpr RegAddr REG_ADDR = RegAddr::IIF_CNT;
    static constexpr uint16_t RESET_VALUE = 0x0000;
    
    uint16_t iif_cnt   : 12; // Bit11-0 增量计数器
    uint16_t res15_12  : 4;  // Bit15-12 保留
};
};


struct TLE5012_Transport final: public TLE5012_Prelude{
public:
    IResult<> write_reg(const RegAddr reg_addr, const uint16_t reg_val);
    IResult<> read_reg(const RegAddr reg_addr, uint16_t & reg_val);
private:

};

class TLE5012 final{
    

private:
    TLE5012_Regset regs_;
};


}