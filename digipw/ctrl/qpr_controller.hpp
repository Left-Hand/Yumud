#pragma once

namespace ymd::digipw{


// QPR控制器
// https://github.com/SSC202/Three_Port_ACDC_Converter/blob/main/Software/Controller/my_math.c
struct QPRController {

    struct Coeffs{
        float d0;
        float a1;
        float a2;
        float b0;
        float b1;
        float b2;
    };


    struct Config{
        float kp; // 直流增益
        float kr; // 谐振增益
        float wr; // 谐振频率
        float wc; // 谐振峰带宽
        float t_sample;

        constexpr Coeffs to_coeffs() const {
            const float d0 = (t_sample * t_sample * wr * wr) + (4 * wc * t_sample) + 4;

            const float a1 = (2 * t_sample * t_sample * wr * wr - 8) / d0;
            const float a2 = (t_sample * t_sample * wr * wr - 4 * wc * t_sample + 4) / d0;

            const float b0 = (
                4 * kp + t_sample * t_sample * kp * wr * wr + 
                4 * t_sample * kp * wc + 4 * t_sample * kr * wc) / d0;

            const float b1 = (
                2 * kp * t_sample * t_sample * wr * wr - 8 * kp) / d0;

            const float b2 = (
                4 * kp + t_sample * t_sample * kp * wr * wr - 
                4 * t_sample * kp * wc - 4 * t_sample * kr * wc) / d0;

            return Coeffs{d0, a1, a2, b0, b1, b2};
        }
    };


    constexpr explicit QPRController(const Config & cfg){

        const auto coeffs = cfg.to_coeffs();

        d0 = coeffs.d0;
        a1 = coeffs.a1;
        a2 = coeffs.a2;
        b0 = coeffs.b0;
        b1 = coeffs.b1;
        b2 = coeffs.b2;

        u[0] = 0;
        u[1] = 0;
        u[2] = 0;
        y[0] = 0;
        y[1] = 0;
        y[2] = 0;

        output = 0;
    }

    constexpr float operator()(const float err){
        u[2] = u[1];
        u[1] = u[0];
        u[0] = err;

        y[2] = y[1];
        y[1] = y[0];
        y[0] = b0 * u[0] + b1 * u[1] + b2 * u[2] - a1 * y[1] - a2 * y[2];

        output = y[0];
        return output;
    }

    constexpr void reset(){
        output = 0;

        u[0] = 0;
        u[1] = 0;
        u[2] = 0;
        y[0] = 0;
        y[1] = 0;
        y[2] = 0;
    }

private:
    // 传递函数系数
    float d0;
    float a1;
    float a2;
    float b0;
    float b1;
    float b2;

    // 临时变量
    float u[3];
    float y[3];

    float output; // 输出值
};
}