#include "apps.h"

#include "src/device/CommonIO/Led/WS2812/ws2812.hpp"
#include "src/device/Display/MonoChrome/SSD1306/ssd1306.hpp"
#include "fwwb_compents/stations/pedestrian_station.hpp"
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


void IWDG_Feed_Init(u16 prer, u16 rlr)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(prer);
	IWDG_SetReload(rlr);
	IWDG_ReloadCounter();
	IWDG_Enable();
}

void pedestrian_app(){
    uart2.init(115200, Uart::Mode::TxOnly);
    logger.setEps(4);
    logger.setRadix(2);
    logger.setSpace(",");


    I2cSw               i2csw(portD[1], portD[0]);
    i2csw.init(0);

    timer3.init(3000);
    timer3[1].setPolarity(false);
    timer3[2].setPolarity(false);

    auto & trigGpioL    (portA[5]);
    auto & trigGpioR    (portB[1]);

    ExtiChannel         trigExtiCHL(trigGpioL, NvicPriority(1, 3), ExtiChannel::Trigger::Rising);
    ExtiChannel         trigExtiCHR(trigGpioR, NvicPriority(1, 4), ExtiChannel::Trigger::Rising);
    CaptureChannelExti  capL(trigExtiCHL, trigGpioL);
    CaptureChannelExti  capR(trigExtiCHR, trigGpioR);

    PwmChannel          pwmL(timer3[1]);
    PwmChannel          pwmR(timer3[2]);

    GM25                motorL(pwmL, portA[1], capL, true);
    GM25                motorR(pwmR, portB[8], capR, false);

    auto &              trigGpioA(portA[0]);
    ExtiChannel         trigExtiCHA(trigGpioA, NvicPriority(1, 0), ExtiChannel::Trigger::RisingFalling);
    CaptureChannelExti  capA(trigExtiCHA, trigGpioA);
    SimpleDeModem       demodemA(capA, mo_freq);
    PanelTarget         panelTargetA(demodemA, 2, 0);


    auto & trigGpioB    (portA[4]);
    ExtiChannel         trigExtiCHB(trigGpioB, NvicPriority(1, 1), ExtiChannel::Trigger::RisingFalling);
    CaptureChannelExti  capB(trigExtiCHB, trigGpioB);
    SimpleDeModem       demodemB(capB, mo_freq);
    PanelTarget         panelTargetB(demodemB, 2, 1);


    delay(200);

    I2cDrv aw_drv(i2csw, AW9523::default_id);
    AW9523 aw(aw_drv);
    aw.init();

    OledInterfaceI2c oled_interface(i2csw, SSD13XX::default_id);
    SSD13XX_128X32
    oled(oled_interface);
    oled.init();
    oled.enableFlipX(false);
    oled.enableFlipY(false);

    Painter<Binary> painter;
    painter.bindImage(oled.fetchFrame());
    // oled.fetchFrame().putpixel({0, 0}, true);
    painter.setColor(true);
    painter.drawString({14, 2}, "Rst");
    oled.update();
    // painter.drawChar({20, 4}, '&');
    // for(int i = 0; i < 7; i ++){
        // oled.putseg_v8_unsafe(Vector2i(i, 0), font8x6_enc[6][i], true);
    // }
    // painter.drawHriLine(Vector2i{20, 2}, 8);
    // painter.drawHriLine(Rangei{17, 28}, 9);
    // painter.drawVerLine(Vector2i{14, 5}, 8);
    // painter.draw

    oled.update();

    AW9523RgbLed        awled_l(aw, Pin::_8, Pin::_10, Pin::_9);
    PanelLed            panelLedA(awled_l);
    AW9523RgbLed        awled_r(aw, Pin::_11, Pin::_1, Pin::_0);
    PanelLed            panelLedB(awled_r);
    
    AW9523Pwm           awpwm(aw, Pin::_11);
    awpwm.init();

    GpioVirtual         ir_left(aw, Pin::_4);
    GpioVirtual         ir_right(aw, Pin::_5);
    GpioVirtual         beep(aw, Pin::_6);
    GpioVirtual         coil_left(aw, Pin::_2);
    GpioVirtual         coil_right(aw, Pin::_15);
    ir_left.InFloating();
    ir_right.InFloating();
    beep.OutPP(true);
    coil_left.OutPP(true); 
    coil_right.OutPP(true);

    I2cDrv              vl_drv(i2csw, VL53L0X::default_id);
    VL53L0X             vl(vl_drv);
    vl.init();
    vl.enableContMode();
    vl.enableHighPrecision();
    vl.startConv();

    I2cDrv qmc_drv(i2csw, 0x1a);
    QMC5883L qmc(qmc_drv);
    qmc.init();

    PanelUnit           panelUnitA(panelTargetA, panelLedA);
    PanelUnit           panelUnitB(panelTargetB, panelLedB); 
    CanStation          can_station(can1, logger);
    TargetStation       target_station(can_station, panelUnitA, panelUnitB);
    DiffPedestrianStation   station(target_station, vl, qmc, ir_left, ir_right, coil_left,coil_right,motorL, motorR);
    // station.init();

    while(true){
        oled.flush(false);
        painter.drawChar(Vector2i{14 + 16 * sin(t), 0}, 'h');
        // painter.drawString(Vector2i{0, 0}, String(frac(t)));
        oled.update();
        // oled.flush(false);
        // station.run();
        // delay(200);
        // station.setOmega(real_t(6 * frac(t)));
        // logger.println(int(t * 1000));

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