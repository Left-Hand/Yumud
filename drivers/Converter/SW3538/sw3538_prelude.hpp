#pragma once

#include <cstdint>
#include "core/io/regs.hpp"

namespace ymd::drivers{

struct SW3538_Prelude {
enum class [[nodiscard]] PdProtocolVersion:uint8_t{
    V2 = 0x01,
    V3 = 0x02
};

enum class [[nodiscard]] FastChargeProtocol:uint8_t{
    QC2 = 0x01,
    QC3 = 0x02,
    FCP,
    SCP,
    PD_FIX,
    PD_PPS,
    PE_1_1,
    PE_2_0,
    LVDC,
    SFCP,
    AFC
};

enum class [[nodiscard]] AdcDataType:uint8_t{
    Vin = 0x01,
    Vout,
    Iout1,
    Iout2,
    Tmp
};

};


struct SW3538_Regset:public SW3538_Prelude {
struct R8_PwrStatus:public Reg8<> { 
    uint8_t ctrl_1port_on:1;
    uint8_t ctrl_2port_on:1;
    uint8_t ctrl_buck_on:1;
    uint8_t :5;
};

struct R8_I2cEnable:public Reg8<> { 
    uint8_t :5;
    uint8_t i2c_pre_enable1:1;
    uint8_t i2c_pre_enable2:1;
    uint8_t i2c_pre_enable:1;
};

struct R8_I2cCtrl:public Reg8<> { 
    uint8_t reg_pd_src_cap_change:1;
    uint8_t reg_adc_vin_enable:1;
    uint8_t :6;
};

struct R8_ProConf:public Reg8<> { 
    uint8_t pd_src_3p0_enable:1;
    uint8_t pd_src_emk_enable:1;
    uint8_t pd_src_f09v_enable:1;
    uint8_t pd_src_f12v_enable:1;
    uint8_t pd_src_f15v_enable:1;
    uint8_t pd_src_f20v_enable:1;
    uint8_t pd_src_pps0_enable:1;
    uint8_t pd_src_pps1_enable:1;
};

struct R8_PdoConf0:public Reg8<> { 
    uint8_t pd_src_cfg_cur_f05v:5;
    uint8_t pd_src_cfg_cur_enable:1;
    uint8_t :1;
    uint8_t pd_src_ctrl_rp_enable:1;
};

struct R8_PdoConf1:public Reg8<> { 
    uint8_t pd_src_cfg_cur_f09v:5;
    uint8_t pd_sc_cfg_cur_pps0:3;
};

struct R8_PdoConf2:public Reg8<> { 
    uint8_t pd_src_cfg_cur_f12v:5;
    uint8_t pd_sc_cfg_cur_pps0:3;
};

struct R8_PdoConf3:public Reg8<> { 
    uint8_t pd_src_cfg_cur_f15v:5;
    uint8_t pd_sc_cfg_cur_pps1:3;
};


struct R8_PdoConf4:public Reg8<> { 
    uint8_t pd_src_cfg_cur_f20v:5;
    uint8_t pd_sc_cfg_cur_pps1:3;
};

enum class OutputPowerSet:uint8_t{
    _18W = 0,
    _24W = 1,
    _36W = 2,
    _45W = 2,
    _60W = 3
};

struct R8_QuickChargeConf0:public Reg8<> { 
    uint8_t pd_src_cfg_vol_pps1:2;
    uint8_t :3;
    uint8_t output_power_set:2;
    uint8_t afc_src_enable:1;
};

struct R8_QuickChargeConf1:public Reg8<> { 
    uint8_t pe_src_enable:1;
    uint8_t lvdc_src_enable:1;
};


};


};