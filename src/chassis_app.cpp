#include "apps.h"

#include "src/device/CommonIO/Led/WS2812/ws2812.hpp"

#include "fwwb_compents/stations/chassis_station.hpp"
#include "src/system.hpp"


#include "dsp/constexprmath/ConstexprMath.hpp"

constexpr uint16_t mo_freq = 3800;

static Printer & logger = uart2;
using namespace FWWB;
using namespace Sys::Clock;

int my_atan5(int x, int y){
    using cem = ConstexprMath;
    constexpr std::array<uint16_t, 15> angle{[]{
        std::array<uint16_t, 15> temp = {};
        for(int i = 0; i < 15; ++i) {
            temp[i] = (uint16_t)(cem::tan(cem::pow(2.0, double(-i))) * (1 << 14));
        }
        return temp;
    }()};

    // constexpr std::array<real_t, 15> si{[]{
    //     std::array<real_t, 15> temp = {};
    //     for(int i = 0; i < 15; ++i) {
    //         temp[i] = real_t(cem::sin(PI / 4 * cem::pow(2.0, double(-i))));
    //     }
    //     return temp;
    // }()};

    int i = 0;
    int x_new, y_new;
    int angleSum = 0;
    x *= 1024;
    y *= 1024;

    for(i = 0; i < 15; i++)
    {
        if(y > 0){
            x_new = x + (y >> i);
            y_new = y - (x >> i);
            x = x_new;
            y = y_new;
            angleSum += angle[i];
        }
        else
        {
            x_new = x - (y >> i);
            y_new = y + (x >> i);
            x = x_new;
            y = y_new;
            angleSum -= angle[i];
        }
        // printf("Debug: i = %d angleSum = %d, angle = %d\n", i, angleSum, angle[i]);  
    }
    return angleSum;
}


template <typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

// 使用概念来定义一个模板函数
template <typename T>
requires Addable<T>
T sum(T a, T b) {
    return a + b;
}

void IWDG_Feed_Init(u16 prer, u16 rlr)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(prer);
	IWDG_SetReload(rlr);
	IWDG_ReloadCounter();
	IWDG_Enable();
}

void chassis_app(){
    uart2.init(115200, Uart::Mode::TxOnly);
    logger.setEps(4);
    logger.setSpace(",");
    logger.println("chassis power on");

    auto & i2c_scl = portD[1];
    auto & i2c_sda = portD[0];
    auto i2csw = I2cSw(i2c_scl, i2c_sda);
    i2csw.init(0);

    delay(20);

    auto aw_drv = I2cDrv(i2csw, 0b10110000);
    auto aw = AW9523(aw_drv);
    aw.init();


    auto awled_r = AW9523RgbLed(aw, Pin::_11, Pin::_1, Pin::_0);
    auto awled_l = AW9523RgbLed(aw, Pin::_8, Pin::_10, Pin::_9);
    auto awpwm = AW9523Pwm(aw, Pin::_11);
    awpwm.init();
    auto ir_left = GpioVirtual(&aw, Pin::_4);
    auto ir_right = GpioVirtual(&aw, Pin::_5);
    auto beep = GpioVirtual(&aw, Pin::_6);
    auto coil_left = GpioVirtual(&aw, Pin::_2);
    auto coil_right = GpioVirtual(&aw, Pin::_15);
    ir_left.InFloating();
    ir_right.InFloating();
    beep.OutPP(); beep.set();
    coil_left.OutPP(); coil_left.set();
    coil_right.OutPP(); coil_right.set();

    auto vl_drv = I2cDrv(i2csw, 0x52);
    auto vl = VL53L0X(vl_drv);

    vl.init();
    vl.setContinuous(true);
    vl.setHighPrecision(false);
    vl.startConv();

    auto qmc_drv = I2cDrv(i2csw, 0x1a);
    auto qmc = QMC5883L(qmc_drv);

    qmc.init();

    auto & trigGpioA = portA[0];
    auto trigExtiCHA = ExtiChannel(trigGpioA, NvicPriority(1, 0), ExtiChannel::Trigger::RisingFalling);
    auto capA = CaptureChannelExti(trigExtiCHA, trigGpioA);
    auto demodemA = SimpleDeModem(capA, mo_freq);
    auto panelTargetA = PanelTarget(demodemA, 2, 0);

    auto panelLedA = PanelLed(awled_l);


    auto & trigGpioB = portA[4];
    auto trigExtiCHB = ExtiChannel(trigGpioB, NvicPriority(1, 1), ExtiChannel::Trigger::RisingFalling);
    auto capB = CaptureChannelExti(trigExtiCHB, trigGpioB);
    auto demodemB = SimpleDeModem(capB, mo_freq);
    auto panelTargetB = PanelTarget(demodemB, 2, 1);

    auto panelLedB = PanelLed(awled_r);

    auto panelUnitA = PanelUnit(panelTargetA, panelLedA);
    auto panelUnitB = PanelUnit(panelTargetB, panelLedB);

    auto & trigGpioL = portA[5];
    auto & trigGpioR = portB[1];

    auto trigExtiCHL = ExtiChannel(trigGpioL, NvicPriority(1, 3), ExtiChannel::Trigger::Rising);
    auto trigExtiCHR = ExtiChannel(trigGpioR, NvicPriority(1, 4), ExtiChannel::Trigger::Rising);
    auto capL = CaptureChannelExti(trigExtiCHL, trigGpioL);
    auto capR = CaptureChannelExti(trigExtiCHR, trigGpioR);

    timer3.init(3000);
    timer3[1].setPolarity(false);
    timer3[2].setPolarity(false);

    auto pwmL = PwmChannel(timer3[1]);
    auto pwmR = PwmChannel(timer3[2]);

    auto motorL = GM25(pwmL, portA[1], capL, true);
    auto motorR = GM25(pwmR, portB[8], capR, false);

    auto can_station = CanStation(can1, logger);
    auto target_station = TargetStation(can_station, panelUnitA, panelUnitB);
    auto station = DiffChassisStation(target_station,
        vl, qmc, ir_left, ir_right, coil_left,coil_right,motorL, motorR);

    // timer4.init(3000);
    // timer4.enableIt(Timer::IT::Update, NvicPriority(1, 7));
    // timer4.bindCb(Timer::IT::Update, [&beep](){
    //     beep= !beep;
    // });

    station.init();
    logger.println("init done");
    // IWDG_Feed_Init( IWDG_Prescaler_32, 4000 );
    // station.setMode(1);
    // NVIC_SystemReset();

    while(true){
        station.run();
        // IWDG_ReloadCounter();
        // if(millis() > 60){
        //     delay(1);
        //     RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, DISABLE);
        //     RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, DISABLE);
        //     __disable_irq();
        //     __nopn(20);
        //     NVIC_SystemReset();
        // }
        // station.setMove(Vector2(real_t(0.1) + 0.1 * sin(t), real_t()));
        // logger.println(TIM3->CH1CVR, TIM3->CH2CVR);
        // logger.println(bool(pp));
        // station.setMove(Vector2());
        // delay(1);
        // beep = !beep;
        // qmc.update();
        // real_t x, y, z;
        // qmc.getMagnet(x, y, z);
        // real_t ang;
        // constexpr double s = ConstexprMath::acos(0.9999);
        // constexpr int t = ConstexprMath::double_factorial().double_factorials[5];
        // constexpr double t = ConstexprMath::asin(1.9999);
        // constexpr int n = ConstexprMath::double_factorial().double_factorials[5];
        // ang.value = my_atan5(x.value, y.value) << 8;
        // auto a = sin(1.0);
        // logger.println(x.value, y, frac(t));
        // logger.println(atan2(x, y));
        // logger.println(capL.getFreq(), ',', capR.getFreq());
        // while(!can1.available());
        // if(millis() % 16 == 0)leds.refresh();
        // delay(22);
        // motorL.setOmega(288 * sin(t));
        // motorR.setOmega(288 * cos(t));
        // motorR.setOmega(400 * sin(t));
        reCalculateTime();
    }
};