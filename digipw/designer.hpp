#pragma once

#include "ctrl/pi_controller.hpp"

namespace ymd::digipw{
struct [[nodiscard]] BuckCurrentRegulatorConfig {
    uint32_t fs;                 // 采样频率 (Hz)
    uint32_t fc;                 // 电流环截止频率 (Hz)
    q20 inductance;              // 输出电感 (H)
    q20 resistance;              // 电感串联电阻 (Ω)
    q20 max_duty;                // 最大占空比 (通常为1.0或略小于1)
    q20 vin_nominal;             // 标称输入电压 (V)

    [[nodiscard]] constexpr digipw::PiController make_pi_controller() const {
        const auto & self = *this;
        digipw::PiController::Cofficients coeff;
        coeff.max_out = self.max_duty;  // 输出为占空比
        
        // 电流环设计：基于电感模型
        // 被控对象传递函数：G(s) = 1 / (L*s + R)
        // q12 omega_bw = q12(TAU) * self.fc;
        
        // Kp = L * ω_c / Vin，用于补偿电感带来的相位滞后
        coeff.kp = (q20(self.inductance) * q20(TAU) * self.fc) / self.vin_nominal;
        
        // Ki = R * ω_c / (Vin * fs)，用于消除稳态误差
        coeff.ki_discrete = (q16(q16(self.resistance) * q16(TAU)) * self.fc) / 
                           (q16(self.vin_nominal) * self.fs);

        // 积分抗饱和限制，基于最大电流和电感电阻
        // 最大误差积分 ≈ 最大电流 / (Ki * fs)
        q20 max_current = self.vin_nominal / self.resistance;  // 估算最大电流
        coeff.err_sum_max = q24(max_current * self.fs / (coeff.ki_discrete * self.fs));
        
        return digipw::PiController(coeff);
    }
};

struct [[nodiscard]] BoostPfcVoltageRegulatorConfig {
    uint32_t fs;                 // 采样频率 (Hz)
    uint32_t fc;                 // 电压环带宽 (Hz)，通常为10-20Hz
    q20 output_capacitance;      // 输出电容 (F)
    q20 vout_nominal;            // 额定输出电压 (V)
    q20 vin_min;                 // 最小输入电压 (V)
    q20 max_current_ref;         // 最大电流参考值 (A)
    
    [[nodiscard]] constexpr digipw::PiController make_pi_controller() const {
        const auto & self = *this;
        digipw::PiController::Cofficients coeff;
        coeff.max_out = self.max_current_ref;
        
        // 基于输出电容能量平衡设计
        // 电压环主要响应较慢，保持稳定
        // q20 power_max = self.vout_nominal * self.max_current_ref;
        // q20 capacitor_energy = q20(0.5) * self.output_capacitance * 
        //                      (self.vout_nominal * self.vout_nominal);
        
        // 经验公式：Kp = 2 * π * fc * C / 2
        coeff.kp = (q20(TAU) * self.fc * self.output_capacitance) / q20(2);
        
        // Ki 设置得更小以保证稳定性
        coeff.ki_discrete = coeff.kp * self.fc / (20 * self.fs);
        
        // 积分抗饱和基于最大电流参考
        coeff.err_sum_max = q24(self.max_current_ref * 5);
        
        return digipw::PiController(coeff);
    }
};

struct [[nodiscard]] GridTieInverterConfig {
    uint32_t fs;                 // 采样频率 (Hz)
    uint32_t fc_current;         // 电流环带宽 (Hz)，通常500-2000Hz
    uint32_t fc_voltage;         // 电压环带宽 (Hz)，通常20-100Hz
    q20 dc_capacitance;          // 直流母线电容(F)
    q20 filter_inductance;       // 滤波电感 (H)
    q20 filter_resistance;       // 滤波电阻 (Ω)
    q20 dc_link_voltage;         // 直流母线电压 (V)
    q20 grid_voltage_max;        // 电网电压峰值 (V)
    q20 max_current;             // 最大并网电流 (A)
    

    [[nodiscard]] constexpr digipw::PiController make_current_regulators() const {
        const auto & self = *this;
        digipw::PiController::Cofficients coeff;
        coeff.max_out = self.dc_link_voltage / q20(2);  // 最大输出电压
        
        coeff.kp = q20(self.filter_inductance) * q20(TAU) * self.fc_current;
        coeff.ki_discrete = q16(q16(self.filter_resistance) * q16(TAU)) * 
            self.fc_current / self.fs;
        
        coeff.err_sum_max = q24(self.max_current * self.fs / (coeff.ki_discrete * self.fs));
        
        return digipw::PiController(coeff);
    }
    
    // 直流母线电压环
    [[nodiscard]] constexpr digipw::PiController make_voltage_regulator() const {
        const auto & self = *this;
        digipw::PiController::Cofficients coeff;
        coeff.max_out = self.max_current;  // 输出为d轴电流参考
        
        coeff.kp = (q20(TAU) * self.fc_voltage * dc_capacitance) / q20(2);
        coeff.ki_discrete = coeff.kp * self.fc_voltage / (10 * self.fs);
        
        coeff.err_sum_max = q24(self.max_current * 3);
        
        return digipw::PiController(coeff);
    }
};

struct [[nodiscard]] LlcFrequencyRegulatorConfig {
    uint32_t fs;                 // 采样频率 (Hz)  
    uint32_t fc;                 // 控制带宽 (Hz)，通常100-500Hz
    q20 min_frequency;           // 最小开关频率 (Hz)
    q20 max_frequency;           // 最大开关频率 (Hz)
    q20 resonant_frequency;      // 谐振频率 (Hz)
    q20 max_voltage_gain;        // 最大电压增益
    
    [[nodiscard]] constexpr digipw::PiController make_pi_controller() const {
        const auto & self = *this;
        digipw::PiController::Cofficients coeff;
        coeff.max_out = self.max_frequency;
        
        // LLC频率控制相对较慢，主要调节增益
        // 经验参数，需要根据具体LLC特性调整
        coeff.kp = (self.max_frequency - self.min_frequency) / 
                  (self.max_voltage_gain * q20(2));
        coeff.ki_discrete = coeff.kp * self.fc / (5 * self.fs);
        
        // 积分限制保证频率在合理范围内
        coeff.err_sum_max = q24((self.max_frequency - self.min_frequency) * 2);
        
        return digipw::PiController(coeff);
    }
};


struct [[nodiscard]] BatteryChargerConfig {
    uint32_t fs;
    uint32_t fc_current;         // 恒流环带宽
    uint32_t fc_voltage;         // 恒压环带宽  
    q20 inductance;
    q20 resistance;
    q20 max_charge_current;
    q20 max_charge_voltage;
    q20 battery_capacity;        // 电池容量 (Ah)
    
    // 恒流模式控制器
    [[nodiscard]] constexpr digipw::PiController make_cc_controller() const {
        const auto & self = *this;
        digipw::PiController::Cofficients coeff;
        coeff.max_out = self.max_charge_voltage;  // 输出为电压限制
        
        coeff.kp = q20(self.inductance) * q20(TAU) * self.fc_current;
        coeff.ki_discrete = q16(q16(self.resistance) * q16(TAU)) * 
                        self.fc_current / self.fs;
        
        coeff.err_sum_max = q24(self.max_charge_current * 10);
        
        return digipw::PiController(coeff);
    }
    
    // 恒压模式控制器  
    [[nodiscard]] constexpr digipw::PiController make_cv_controller() const {
        const auto & self = *this;
        digipw::PiController::Cofficients coeff;
        coeff.max_out = self.max_charge_current;  // 输出为电流限制
        
        // 基于电池内阻和电容效应
        q20 battery_impedance = q20(0.01);  // 典型电池内阻
        coeff.kp = q20(TAU) * self.fc_voltage / battery_impedance;
        coeff.ki_discrete = coeff.kp * self.fc_voltage / (8 * self.fs);
        
        coeff.err_sum_max = q24(self.max_charge_current * 2);
        
        return digipw::PiController(coeff);
    }
};
}