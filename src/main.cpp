#include "misc.h"

using Complex = Complex_t<real_t>;
using Color = Color_t<real_t>;


Gpio i2cScl = Gpio(GPIOB, I2C_SW_SCL);
Gpio i2cSda = Gpio(GPIOB, I2C_SW_SDA);

// Gpio i2sSck = Gpio(GPIOB, I2S_SW_SCK);
// Gpio i2sSda = Gpio(GPIOB, I2S_SW_SDA);
// Gpio i2sWs = Gpio(GPIOB, I2S_SW_WS);

I2cSw i2cSw(i2cScl, i2cSda);
// I2sSw i2sSw(i2sSck, i2sSda, i2sWs);

// SpiDrv SpiDrvLcd = SpiDrv(spi2_hs, 0);
// SpiDrv spiDrvOled = SpiDrv(spi2, 0);
// SpiDrv spiDrvFlash = SpiDrv(spi1, 0);
// SpiDrv spiDrvMagSensor = SpiDrv(spi1, 0);
// SpiDrv spiDrvRadio = SpiDrv(spi1, 0);
// I2cDrv i2cDrvOled = I2cDrv(i2cSw,(uint8_t)0x78);
// I2cDrv i2cDrvMpu = I2cDrv(i2cSw,(uint8_t)0xD0);
// I2cDrv i2cDrvAdc = I2cDrv(i2c1, 0x90);
// I2cDrv i2cDrvTcs = I2cDrv(i2cSw, 0x52);
// I2cDrv i2cDrvVlx = I2cDrv(i2cSw, 0x52);
// I2cDrv i2cDrvPcf = I2cDrv(i2cSw, 0x4e);
// I2cDrv i2cDrvAS = I2cDrv(i2cSw, 0x6c);
// I2sDrv i2sDrvTm = I2sDrv(i2sSw);
I2cDrv i2cDrvAS = I2cDrv(i2cSw, 0x36 << 1);
I2cDrv i2cDrvQm = I2cDrv(i2cSw, 0x1a);
I2cDrv i2cDrvBm = I2cDrv(i2cSw, 0xec);
// ST7789 tftDisplayer(SpiDrvLcd);
// SSD1306 oledDisPlayer(spiDrvOled);
// MPU6050 mpu(i2cDrvMpu);
// SGM58031 ext_adc(i2cDrvAdc);
// LT8920 radio(spiDrvRadio);
// TCS34725 tcs(i2cDrvTcs);
// VL53L0X vlx(i2cDrvVlx);
// PCF8574 pcf(i2cDrvPcf);
// AS5600 mags(i2cDrvAS);
// TM8211 extern_dac(i2sDrvTm);
// W25QXX extern_flash(spiDrvFlash);
// MA730 mag_sensor(spiDrvMagSensor);
AS5600 mag_sensor(i2cDrvAS);
QMC5883L earth_sensor(i2cDrvQm);
BMP280 prs_sensor(i2cDrvBm);

Gpio blueLed = Gpio(BUILTIN_LED_PORT, BUILTIN_RedLED_PIN);

extern "C" void TimBase_IRQHandler(void) __interrupt;

RGB565 color = 0xffff;
const RGB565 white = 0xffff;
const RGB565 black = 0;
const RGB565 red = RGB565(31,0,0);
const RGB565 green = RGB565(0,63,0);
const RGB565 blue = RGB565(0,0,31);

real_t delta = real_t(0);
real_t fps = real_t(0);
real_t t = real_t(0);

namespace Shaders{
__fast_inline RGB565 ShaderP(const Vector2i & pos){
    static int cnt = 0;
    cnt++;
    return pos.x + pos.y + cnt;
}

__fast_inline RGB565 ShaderUV(const Vector2 & UV){
    if((UV - Vector2(0.5, 0.5)).length_squared() <= real_t(0.25)){
        return RGB565::RED;
    }else{
        return RGB565::BLUE;
    }
}

__fast_inline RGB565 Mandelbrot(const Vector2 & UV){
    Complex c(lerp(UV.x, real_t(0.5), real_t(-1.5)), lerp(UV.y, real_t(-1), real_t(1)));
    Complex z;
    uint8_t count = 0;

    while ((z < real_t(4)) && (count < 23))
    {
        z = z*z + c;
        count = count + 1;
    }
    return count * 100;
}
};


constexpr uint16_t pwm_arr = 144000000/72000 - 1;
constexpr uint16_t ir_arr = 144000000/38000 - 1;

struct MotorPosition{
    real_t lapPositionHome = real_t(0);
    real_t lapPosition = real_t(0);
    real_t lapPositionLast = real_t(0);

    int16_t accTurns = 0;
    real_t accPosition = real_t(0);
    // real_t accPositionFixed = real_t(0);
    real_t accPositionLast = real_t(0);

    real_t vel = real_t(0);
    real_t accPositionAgo = real_t(0);
    real_t elecRad = real_t(0);
    // real_t elecRadFixed = real_t(0);
}motorPosition;
real_t readLapPosition(){
    real_t ret;
    u16_to_uni((uint16_t)(TIM2->CNT), ret);
    return ret;
}
void updatePosition(){
    motorPosition.lapPosition = readLapPosition();
    real_t deltaLapPosition = motorPosition.lapPosition - motorPosition.lapPositionLast;

    if(deltaLapPosition > real_t(0.5f)){
        motorPosition.accTurns -= 1;
    }else if (deltaLapPosition < real_t(-0.5f)){
        motorPosition.accTurns += 1;
    }

    motorPosition.lapPositionLast = motorPosition.lapPosition;
    motorPosition.accPositionLast = motorPosition.accPosition;
    motorPosition.accPosition = real_t(motorPosition.accTurns) + (motorPosition.lapPosition - motorPosition.lapPositionHome);
}
void setMotorDuty(const real_t & duty){
    if(duty >= real_t(0)){
        uint16_t value = (int)(duty * pwm_arr);
        TIM4->CH1CVR = value;
        TIM4->CH2CVR = 0;
        TIM4->CH3CVR = value / 2;
    }else{
        uint16_t value = (int)((-duty) * pwm_arr);
        TIM4->CH1CVR = 0;
        TIM4->CH2CVR = value;
        TIM4->CH3CVR = value / 2;
    }
}

void setABCoilDuty(const real_t & aduty, const real_t & bduty){
    if(aduty >= real_t(0)){
        uint16_t value = (int)(aduty * pwm_arr);
        TIM4->CH1CVR = value;
        TIM4->CH2CVR = 0;
    }else{
        uint16_t value = (int)((-aduty) * pwm_arr);
        TIM4->CH1CVR = 0;
        TIM4->CH2CVR = value;
    }

    if(bduty >= real_t(0)){
        uint16_t value = (int)(bduty * pwm_arr);
        TIM4->CH3CVR = value;
        TIM4->CH4CVR = 0;
    }else{
        uint16_t value = (int)((-bduty) * pwm_arr);
        TIM4->CH3CVR = 0;
        TIM4->CH4CVR = value;
    }
}


void setIrState(const bool on){
    TIM3->CH1CVR = on ? (ir_arr / 3) : 0;
}



class NecEncoder{
protected:
	uint8_t bit_prog;
	uint8_t byte_prog;

	struct NecCode{
		uint8_t total_cnt;
		uint8_t valid_cnt;
	};

	const NecCode codes[4] = {
		{.total_cnt = 24, .valid_cnt = 16},
		{.total_cnt = 2, .valid_cnt = 1},
		{.total_cnt = 4, .valid_cnt = 1},
		{.total_cnt = 2, .valid_cnt = 1}
	};

	enum class BitType:uint8_t{
		Leader, Zero, One, Stop
	};

	enum class EncodeProg:uint8_t{
		Idle, Lead, Address, invAddress, Command, invCommand, Stop
	};

	bool writeBit(const BitType & bit){
		bit_prog ++;
		const NecCode & code = codes[(uint8_t)bit];

		setIrState(bit_prog <= code.valid_cnt);

		bool ret = false;
		if(bit_prog >= code.total_cnt){
			bit_prog = 0;
			ret = true;
		}
		return ret;
	}
	bool writeByte(const uint8_t & byte){

		if(writeBit((byte & (0x01 << byte_prog)) ? BitType::One : BitType::Zero))
			byte_prog ++;

		bool ret = false;

		if(byte_prog >= 8){
			byte_prog = 0;
			ret = true;
		}

		return ret;
	};
public:
	uint8_t address = 0xAA;
	uint8_t command = 0xCC;
	EncodeProg encode_prog;
	bool tick(){
		switch(encode_prog){
		case EncodeProg::Lead:
			if(writeBit(BitType::Leader)){
				encode_prog = EncodeProg::Address;
			}
			break;

		case EncodeProg::Address:
			if(writeByte(address)){
				encode_prog = EncodeProg::invAddress;
			}
			break;
		case EncodeProg::invAddress:
			if(writeByte(~address)){
				encode_prog = EncodeProg::Command;
			}
			break;
		case EncodeProg::Command:
			if(writeByte(command)){
				encode_prog = EncodeProg::invCommand;
			}
			break;
		case EncodeProg::invCommand:
			if(writeByte(~command)){
				encode_prog = EncodeProg::Stop;
			}
			break;
		case EncodeProg::Stop:
			if(writeBit(BitType::Stop)){
				encode_prog = EncodeProg::Idle;
			}
            // setIrState(false);
			break;
		default:
			break;
		}

		return (encode_prog == EncodeProg::Idle);
	}

	void emit(const uint8_t & _address, const uint8_t & _command){
		address = _address;
		command = _command;

		encode_prog = EncodeProg::Lead;
	}
}ir_encoder;

int main(){
    RCC_PCLK1Config(RCC_HCLK_Div1);
    RCC_PCLK2Config(RCC_HCLK_Div1);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    Systick_Init();
    GPIO_SW_I2C_Init();
    // LED_GPIO_Init();
    GPIO_PortC_Init();
    Gpio Led = Gpio(GPIOC, GPIO_Pin_13);

    // GPIO_SW_I2S_Init();

    // TIM2_GPIO_Init();
    // TIM_Encoder_Init(TIM2);

    // TIM4_GPIO_Init();
    // TIM_PWM_Init(TIM4, pwm_arr);

    // TIM3_GPIO_Init();
    // TIM_PWM_Init(TIM3, ir_arr);
    // ADC1_GPIO_Init();
    // ADC1_Init();

    // uart1.init(UART1_Baudrate);
    uart2.init(UART2_Baudrate);
    // uart2.init(576000);
    // can1.init(Can1::BaudRate::Mbps1);
    // i2c1.init(100000);
    // i2c1.setTimeout(320);
    // i2cSw.
    // mag_sensor.init();
    earth_sensor.init();
    prs_sensor.init();
    while(true);
    while(true){
        while(!earth_sensor.isIdle());
        delay(2);
        real_t x, y, z;
        earth_sensor.getMag(x, y, z);
        uart2.println(x, y, z);
        delay(10);
    }
    // can1.enableHwReTransmit();
    // bool tx_role = getChipId() == 6002379527825632205;

    // can1.write(CanMsg(0x11));
    // can1.write(CanMsg(0x14));
    // CanMsg msg_v = CanMsg(0x800, {0x15, 0x16});
    // can1.write(msg_v);
    // while(can1.pending()){
    //     uart2.println(can1.pending(), can1.available());
    // }
    // spi2.init(72000000);
    // spi1.init(18000000);

    // GLobal_Reset();
    // SysInfo_ShowUp(uart2);



    // bool use_tft = true;
    // bool use_mini = false;
    // if(use_tft){
    // if(use_mini){
    //     tftDisplayer.init();
    //     tftDisplayer.setDisplayArea(Rect2i(0, 0, 160, 80));
    //     tftDisplayer.setDisplayOffset(Vector2i(1, 26));
    //     tftDisplayer.setFlipX(true);
    //     tftDisplayer.setFlipY(false);
    //     tftDisplayer.setSwapXY(true);
    //     tftDisplayer.setFormatRGB(false);
    //     tftDisplayer.setFlushDirH(false);
    //     tftDisplayer.setFlushDirV(false);
    //     tftDisplayer.setInversion(true);
    // }else{
    //     tftDisplayer.init();
    //     tftDisplayer.setDisplayArea(Rect2i(0, 0, 240, 240));

    //     tftDisplayer.setFlipX(false);
    //     tftDisplayer.setFlipY(false);
    //     tftDisplayer.setSwapXY(false);
    //     tftDisplayer.setFormatRGB(true);
    //     tftDisplayer.setFlushDirH(false);
    //     tftDisplayer.setFlushDirV(false);
    //     tftDisplayer.setInversion(true);
    // }}else{
    //     oledDisPlayer.init();

    //     oledDisPlayer.setOffsetY(6);
    //     oledDisPlayer.setFlipX(false);
    //     oledDisPlayer.setFlipY(false);
    //     oledDisPlayer.setInversion(false);
    // }

    // mpu.init();
    // ext_adc.init();
    // // print("ext_adc ini")
    // ext_adc.setContMode(true);
    // ext_adc.setFS(SGM58031::FS::FS4_096);
    // ext_adc.setMux(SGM58031::MUX::P0NG);
    // ext_adc.setDataRate(SGM58031::DataRate::DR960);
    // ext_adc.startConv();
    // radio.init();
    // uart1.println("flashCapacity: ", extern_flash.getDeviceCapacity());
    // uint8_t * buf = new uint8_t[uart1.read()];
    // uart1.println(String((const char *)buf));
    // tcs.init();
    // tcs.setIntegration(48);
    // tcs.setGain(TCS34725::Gain::X60);
    // tcs.startConv();
    // vlx.init();
    // vlx.setContinuous(true);
    // vlx.setHighPrecision(false);
    // vlx.startConv();
    // mags.init();
    // extern_dac.setDistort(5);
    // extern_dac.setRail(real_t(1), real_t(4));

    // mag_sensor.setPulsePerTurn(30);
    // Font6x8 font6x8;
    // Painter<RGB565> painter(&tftDisplayer, &font6x8);
    // uart1.setSpace(",");
    // uart2.setSpace(",");

    // painter.setColor(RGB565::BLACK);
    // painter.flush();
    // painter.setColor(RGB565::WHITE);
    // painter.drawString(Vector2i(0,0), String((int16_t)TIM2->CNT));
    // painter.drawString(Vector2i(0,8), String(0.2));
    // painter.drawString(Vector2i(0,16), String(0));
    // painter.drawString(Vector2i(0,24), String(3672));

    uart2.println("initialized");
    while(1){
        // if(tx_role){
        //     static uint8_t cnt = 0;
        //     // CanMsg msg_v = CanMsg(cnt << 4, {cnt, (uint8_t)(cnt + 1)});
        //     uint64_t chipId = getChipId();
        //     CanMsg msg_v = CanMsg(cnt << 4, (uint8_t *)&chipId, 8);

        //     can1.write(msg_v);
        //     uart2.println("tx", msg_v.getId(),msg_v(0), msg_v(1));
        //     while(can1.pending()){
        //         uart2.println("err", can1.getTxErrCnt(), can1.getRxErrCnt(), can1.isBusOff());
        //         delay(2);
        //     }
        //     while(can1.available()){
        //         CanMsg msg_r = can1.read();
        //         uart2.println("rx", msg_r.getId(), msg_r(0), msg_r(1));
        //     }
        //     // uart2.println(cnt++, uart2.available());
        //     Led = !Led;
        //     cnt++;
        //     // delay(1);
        //     delay(20);
        //     // delayMicroseconds(20000);/
        // }else{
        //     while(can1.available()){
        //         CanMsg msg_r = can1.read();
        //         uart2.println("rx", msg_r.getId(), msg_r(0), msg_r(1));
        //     }
        //     // uart2.println(can1.available());
        //     // delay(20);
        // }
        uart2.println(mag_sensor.getRawAngle(), mag_sensor.getMagnitude(), mag_sensor.getMagStatus());
        Led = !Led;
        // updatePosition();
        // real_t pos = motorPosition.accPosition * 10;
        // static PID pos_pid = PID(real_t(10), real_t(0), real_t(0));
        // pos_pid.setClamp(real_t(0.94));

        // real_t target = sin(t);
        // // real_t duty = CLAMP((target - pos) * 30, real_t(-0.94), real_t(0.94));
        // real_t duty = pos_pid.update(target, pos);
        // // real_t duty = floor(fmod(t, real_t(3))-1);
        // // real_t duty = sin(t * real_t(-2));
        // setMotorDuty(duty);
        // real_t position = t;
        // const real_t omiga = real_t(10);
        // const real_t amplitude = real_t(1);
        // real_t rad = floor(omiga * position / real_t(TAU/4)) * real_t(TAU/4);
        // real_t a = sin(rad) * amplitude;
        // real_t b = cos(rad) * amplitude;
        // uart1.println(a,b);
        // setABCoilDuty(a, b);
        // reCalculateTime();
        // blueLed = !blueLed;
        // setIrState((bool)blueLed);
        // if(ir_encoder.tick()){
        //     static uint8_t a = 0x00;
        //     ir_encoder.emit(a++, a);
        // }
        // setIrState(true);
        // delay(2);
        // delayMicroseconds(160);
        // uart1.println(millis());
        reCalculateTime();
        // t = real_t(TAU/4);
        // ADC_SoftwareStartInjectedConvCmd(ADC1,ENABLE);
        // while(ADC_GetFlagStatus(ADC1,ADC_FLAG_JEOC)==RESET);

        // uint16_t ad1 = ADC_GetInjectedConversionValue(ADC1,ADC_InjectedChannel_1);
        // uint16_t ad2 = ADC_GetInjectedConversionValue(ADC1,ADC_InjectedChannel_2);
        // uart1.println(ir_encoder.bit_prog, (uint8_t)ir_encoder.byte_prog, (uint8_t)ir_encoder.encode_prog, TIM3->CH1CVR);
    }
}
