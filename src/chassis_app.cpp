#include "apps.h"

#include "src/device/CommonIO/Led/WS2812/ws2812.hpp"

#include "fwwb_compents/stations/chassis_station.hpp"
#include "src/system.hpp"


#include "dsp/constexprmath/ConstexprMath.hpp"

constexpr uint16_t mo_freq = 3800;

static Printer & logger = uart2;
using namespace FWWB;
using namespace Sys::Clock;

template<typename real, int size>
class Cordic{
protected:
    using cem = ConstexprMath;

	struct Coord{
		real x;
		real y;
	};

	std::array<_iq, size> sine{[]{
        std::array<_iq, size> temp = {};
        for(int i = 0; i < size; ++i) {
            temp[i] = _iq(cem::sin(PI / 4 * cem::pow(2.0, double(-i)))*(1 << GLOBAL_Q));
        }
        return temp;
    }()};

	std::array<_iq, size> cosine{[]{
        std::array<_iq, size> temp = {};
        for(int i = 0; i < size; ++i) {
            temp[i] = _iq(cem::cos(PI / 4 * cem::pow(2.0, double(-i)))*(1 << GLOBAL_Q));
        }
        return temp;
    }()};

	Coord sincosu(const real & _x) const{
		real unit = real(PI / 4);
		real x = fmod(_x, PI);
		Coord coord = Coord{real(1), real(0)};

		for(uint8_t i = 0; i < size; i++){
			if(x > unit){
				coord = Coord{
					coord.x * real_t(cosine[i]) - coord.y * real_t(sine[i]),
					coord.y * real_t(cosine[i]) + coord.x * real_t(sine[i])
				};
				x -= unit;
			}
			unit /= 2;
		}

		return coord;
	}

	Coord atan2squu(const real & _y, const real & _x) const{
		Coord coord = Coord{_x, _y};
		real angleSum = real(0);
		real angle = real(PI / 4);

		for(uint8_t i = 0; i < size; i++){
			if(coord.y > 0){
				coord = Coord{
					coord.x * real_t(cosine[i]) + coord.y * real_t(sine[i]),
					coord.y * real_t(cosine[i]) - coord.x * real_t(sine[i])
				};
				angleSum += angle;
			}
			else{
				coord = Coord{
					coord.x * real_t(cosine[i]) - coord.y * real_t(sine[i]),
					coord.y * real_t(cosine[i]) + coord.x * real_t(sine[i])
				};
				angleSum -= angle;
			}
			angle /= 2;
		}
		return Coord{coord.x, angleSum};
	}

	Coord atan2squ(const real & y, const real & x) const{

		if(x >= 0){
			Coord ret = atan2squu(abs(y), x);
			real atan2_abs = ret.y;
			real squ_abs = ret.x;
			return Coord{squ_abs, atan2_abs > 0 ? atan2_abs : -atan2_abs};
		}else{
			if(y > 0){
				Coord ret = atan2squu(-x, y);
				return Coord{ret.x, ret.y + PI / 2};
			}else{
				Coord ret = atan2squu(-x, -y);
				return Coord{ret.x, - PI / 2 - ret.y};
			}
		}
	}
public:
	consteval Cordic(){;}

	real squ(const real & y, const real & x) const{
		return atan2squ(y,x).x;
	}
	real atan2(const real & y, const real & x) const{
		return atan2squ(y, x).y;
	}

	real asin(const real & x) const {
		if (x <= -1) return -real(PI / 2);
		if (x >= 1) return real(PI / 2);
		return atan2(x, sqrt(1 - x * x));
	}

	real acos(const real & x) const{
		if (x <= -1) return real(PI);
		if (x >= 1) return real(0);
		return atan2(sqrt(1 - x * x), x);
	}

	real sin(const real & x) const{
		real ret = sincosu(x).y;
		return x > 0 ? ret : -ret;
	}

	real cos(const real & x){
		return sincosu(abs(x)).x;
	}

//	real tan(const real & x){
//		Coord coord
//	}
};



// struct anglelut{
//     static constexpr uint16_t size = 14;
//     std::array<uint16_t, size> lut;
// public:
//     anglelut()lut{

//     }
// };

template <typename Func, int N>
struct myArray {
    int data[N];

    constexpr myArray(Func f) : data{} {
        for (int i = 0; i < N; ++i) {
            data[i] = i;
        }
    }
};


static constexpr double hh(double x){
    return ConstexprMath::sin(x);
    // return x * 8.2;
}
template <int N>
struct dbArray {
    std::array<double, N> data;

    constexpr dbArray() : data([](){
        std::array<double, N> temp;
        for (int i = 0; i < N; ++i) {
            temp[i] = hh(i) / N;
        }
        return temp;
        }()) {} // Directly initialize data with the result of the lambda function
};

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

void chassis_app(){
    uart2.init(115200, Uart::Mode::TxOnly);
    logger.setEps(4);
    logger.setSpace(",");
    logger.println("chassis power on");

    can1.init(Can::BaudRate::Mbps1);


    // auto ws_out = portA[-1];
    // auto leds = WS2812Chain<3>(ws_out);
    // leds.init();

    auto i2c_scl = portD[1];
    auto i2c_sda = portD[0];
    auto i2csw = I2cSw(i2c_scl, i2c_sda);
    i2csw.init(0);

    auto aw_drv = I2cDrv(i2csw, 0b10110000);
    auto aw = AW9523(aw_drv);
    aw.init();


    auto awled_r = AW9523RgbLed(aw, Pin::_11, Pin::_1, Pin::_0);
    auto awled_l = AW9523RgbLed(aw, Pin::_8, Pin::_10, Pin::_9);
    auto awpwm = AW9523Pwm(aw, Pin::_11);
    awpwm.init();
    auto ir_left = GpioVirtual(&aw, Pin::_4);
    auto ir_right = GpioVirtual(&aw, Pin::_5);
    ir_left.InFloating();
    ir_right.InFloating();

    auto vl_drv = I2cDrv(i2csw, 0x52);
    auto vl = VL53L0X(vl_drv);

    vl.init();
    vl.setContinuous(true);
    vl.setHighPrecision(false);
    vl.startConv();

    auto qmc_drv = I2cDrv(i2csw, 0x1a);
    auto qmc = QMC5883L(qmc_drv);

    qmc.init();

    auto trigGpioA = portA[0];
    auto trigExtiCHA = ExtiChannel(trigGpioA, NvicPriority(1, 0), ExtiChannel::Trigger::RisingFalling);
    auto capA = CaptureChannelExti(trigExtiCHA, trigGpioA);
    auto demodemA = SimpleDeModem(capA, mo_freq);
    auto panelTargetA = PanelTarget(demodemA, 2, 0);

    auto panelLedA = PanelLed(awled_l);


    auto trigGpioB = portA[4];
    auto trigExtiCHB = ExtiChannel(trigGpioB, NvicPriority(1, 1), ExtiChannel::Trigger::RisingFalling);
    auto capB = CaptureChannelExti(trigExtiCHB, trigGpioB);
    auto demodemB = SimpleDeModem(capB, mo_freq);
    auto panelTargetB = PanelTarget(demodemB, 2, 1);

    auto panelLedB = PanelLed(awled_r);

    auto panelUnitA = PanelUnit(panelTargetA, panelLedA);
    auto panelUnitB = PanelUnit(panelTargetB, panelLedB);

    auto trigGpioL = portA[5];
    auto trigGpioR = portB[1];

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
        vl, ir_left, ir_right, motorL, motorR);
    station.init();
    station.setMode(1);

    // constexpr auto myLambda = [](int x) constexpr {
    //     return x * x;
    // };
    // constexpr myArray array(myLambda);

    constexpr Cordic<real_t, 12> cordic;
    while(true){
        station.run();
        qmc.update();
        real_t x, y, z;
        qmc.getMagnet(x, y, z);
        real_t ang;
        // constexpr double s = ConstexprMath::acos(0.9999);
        // constexpr int t = ConstexprMath::double_factorial().double_factorials[5];
        // constexpr double t = ConstexprMath::asin(1.9999);
        // constexpr int n = ConstexprMath::double_factorial().double_factorials[5];
        // ang.value = my_atan5(x.value, y.value) << 8;
        // auto a = sin(1.0);
        logger.println(cordic.atan2(x, y), x.value, y, frac(t));
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