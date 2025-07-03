#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/realmath.hpp"

#include "hal/bus/i2c/i2csw.hpp"
#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/timer/instance/timer_hw.hpp"

#include "drivers/IMU/Magnetometer/MMC5983/MMC5983.hpp"
#include "robots/gesture/mahony.hpp"
#include "hal/bus/uart/uarthw.hpp"
#include "hal/gpio/gpio_port.hpp"
#include "thirdparty/sstl/include/sstl/vector.h"

using namespace ymd;
using namespace ymd::hal;
using namespace ymd::drivers;

// #define UART uart2
#define UART uart2
#define SCL_GPIO hal::portB[3]
#define SDA_GPIO hal::portB[5]
static constexpr uint FS = 100;
static constexpr auto INV_FS = (1.0_q24 / FS);
// #define MAG_ACTIVATED

// https://zhuanlan.zhihu.com/p/717479974

static constexpr Vector2<q24> project_idx_to_v2(const size_t i, const size_t n){
    const auto r = 1 - 2 * q16(i) / (n - 1);
    // q24 phi = std::cos(r); // 极角
    q24 phi = r; // 极角
    // q24 phi = 0;
    q24 theta = q24(M_PI) * (1.0_q24 + std::sqrt(5.0_q24)) * i; // 方位角
    return {phi, theta};
}
static constexpr Vector3<q24> project_v2_to_v3(Vector2<q24> v2){
    const auto [theta, phi] = v2;
    return {
        cos(theta) * sin(phi),
        sin(theta) * sin(phi),
        cos(phi)
    };
}

static constexpr Vector2<q24> project_v3_to_v2(const Vector3<q24> v3){
    q24 phi = std::acos(v3.z); // 极角
    q24 theta = std::atan2(v3.y, v3.x); // 方位角
    return {theta, phi};
}

static constexpr size_t project_v2_to_idx(const Vector2<q24> v2, const size_t n){
    const auto [theta, phi] = v2;
    q24 i = (1 - cos(phi)) * (n - 1) / 2; // 计算索引
    return static_cast<size_t>(i);
}




static constexpr std::tuple<Vector3<q24>, Vector3<q24>> 
calibrate_magfield(const std::span<const Vector3<q24>> data) {
    // // 1. 计算平均值(初始硬铁偏移估计)

    const q24 inv_size = 1.0_q24 / data.size();


    // const Vector3<q24> mean = std::accumulate(data.begin(), data.end(), Vector3<q24>{0, 0, 0}) / data.size();
    //手动展开循环 避免溢出
    const auto mean = [&]{
        Vector3<q24> sum{0, 0, 0};
        for (const auto & v : data) {
            sum += v * inv_size;
        }
        return sum;
    }();

    // 3. 构建线性方程组系数(避免矩阵运算)

    q24 A[6] = {0}; // 对称矩阵的上三角部分
    q24 b[3] = {0}; // 右侧向量

    for (size_t i = 0; i < data.size(); ++i) {
        const auto x = data[i].x - mean.x;
        const auto y = data[i].y - mean.y;
        const auto z = data[i].z - mean.z;
        
        const auto x2 = (x * x);
        const auto y2 = (y * y);
        const auto z2 = (z * z);

        // 填充对称矩阵
        A[0] += x2 * x2; // A11
        A[1] += x2 * y2; // A12
        A[2] += x2 * z2; // A13
        A[3] += y2 * y2; // A22
        A[4] += y2 * z2; // A23
        A[5] += z2 * z2; // A33

        // 填充右侧向量
        b[0] += x2;
        b[1] += y2;
        b[2] += z2;

        DEBUG_PRINTLN(A, b);
        clock::delay(1ms);
    }


    // 4. 解3x3线性方程组(使用克莱姆法则，避免矩阵求逆)
    auto solve_3x3 = [&]() -> Vector3<q24> {
        // 计算行列式
        const auto det = A[0]*(A[3]*A[5] - A[4]*A[4]) 
                        - A[1]*(A[1]*A[5] - A[4]*A[2]) 
                        + A[2]*(A[1]*A[4] - A[3]*A[2]);

        if (det == 0) {
            return Vector3<q24>{1, 1, 1};
        }

        // 计算各变量的行列式
        const auto det_x = b[0]*(A[3]*A[5] - A[4]*A[4]) 
                            - A[1]*(b[1]*A[5] - A[4]*b[2]) 
                            + A[2]*(b[1]*A[4] - A[3]*b[2]);

        const auto det_y = A[0]*(b[1]*A[5] - A[4]*b[2]) 
                            - b[0]*(A[1]*A[5] - A[4]*A[2]) 
                            + A[2]*(A[1]*b[2] - b[1]*A[2]);

        const auto det_z = A[0]*(A[3]*b[2] - b[1]*A[4]) 
                            - A[1]*(A[1]*b[2] - b[1]*A[2]) 
                            + b[0]*(A[1]*A[4] - A[3]*A[2]);

        return Vector3<q24>{
            det_x / det,
            det_y / det,
            det_z / det
        };
    };

    Vector3<q24> solution = solve_3x3();

    // 5. 计算软铁缩放因子
    auto compute_scale_factors = [](Vector3<q24> params) {
        return Vector3<q24>{
            isqrt(MAX(params.x, 1)),
            isqrt(MAX(params.y, 1)),
            isqrt(MAX(params.z, 1))
        };
    };

    Vector3<q24> soft_iron = compute_scale_factors(solution);

    return {mean, soft_iron};
}

class EllipseCalibrator{
public:
    static constexpr size_t N = 48;
    // using Data = std::array<Vector3<q24>, N>;
    using Data = sstl::vector<Vector3<q24>, N>;

    // struct Flag{
        // Empty,
        // Valid
    // };
    using Flag = uint8_t;

    using Flags = std::array<Flag, N>;

    EllipseCalibrator(){
        flags_.fill(0);
    }

    void add_data(const Vector3<q24> & v3){
        // const auto v2 = project_v3_to_v2(v3.normalized());
        // const auto idx = project_v2_to_idx(v2, N);

        const auto idx8 = [&] -> uint8_t{
            const auto [x,y,z] = v3;
            const uint8_t b1 = std::signbit(x);
            const uint8_t b2 = std::signbit(y);
            const uint8_t b3 = std::signbit(z);
            return b1 << 2 | b2 << 1 | b3;
        }();

        const auto idx6 = [&] -> uint8_t{
            const auto [x0,y0,z0] = v3;
            const auto [x,y,z] = Vector3{ABS(x0), ABS(y0), ABS(z0)};

            const bool b1 = std::signbit(y-z);
            const bool b2 = std::signbit(x-z);
            const bool b3 = std::signbit(x-y);
            switch(b1 << 2 | b2 << 1 | b3){
                case 0b100: return 0;
                case 0b110: return 1;
                case 0b111: return 2;
                case 0b011: return 3;
                case 0b001: return 4;
                case 0b000: return 5;
                default: PANIC(b1, b2, b3);
            }
        }();

        const size_t idx = idx8 * 6 + idx6;
        // if(idx >= N) PANIC(N);
        if(idx < N and is_index_empty(idx)){
            // data_[idx] = v3;
            data_.push_back(v3);
            flags_[idx] = 1;
        }
    }

    auto get_result() const {
        return calibrate_magfield(std::span(data_));
    }

    template<typename T>
    static constexpr T reduce_span(const std::span<const T> data){
            return std::accumulate(data.begin(), data.end(), 0);
    };

    std::array<uint8_t, 8> get_progress() const {
        std::array<uint8_t, 8> ret;

        for(size_t i = 0; i < 48; i+=6){
            ret[i / 6] = reduce_span(std::span(flags_.begin() + i, 6));
        }
        
        return ret;
    }

    uint8_t get_percentage() const {
        const auto prog = get_progress();
        uint8_t sum = 0;
        for(auto v : prog) sum += v;
        return sum;
        // return std::accumulate(prog.begin(), prog.end(), uint8_t(0)) * 100 / 48;
    }


    bool is_index_empty(size_t i){
        return flags_[i] == false;
    }

    std::span<const Vector3<q24>> data() const {return data_;}
private:
    Data data_;
    Flags flags_;
};

static void mmc5983_test(drivers::MMC5983 & imu){

    if(const auto res = imu.init({});
        res.is_err()) PANIC(res.unwrap_err());

    imu.enable_auto_mag_sr(EN).unwrap();

    // DEBUG_PRINTLN(do_set().unwrap());

    // while(true);
    // const auto m = millis();
    const auto up = (imu.do_magreset().unwrap() + imu.do_magreset().unwrap())/ 2;
    const auto down = (imu.do_magset().unwrap() + imu.do_magset().unwrap())/ 2;
    const auto base = (up + down)/2;
    // PANIC(millis() - m);
    // sstl::vector<Vector3<q24>, 64> data;
    EllipseCalibrator calibrator;
    Vector3<q24> mean, soft_iron;

    // for(size_t i = 0; i < 100; i++){
    //     while(imu.is_mag_meas_done().unwrap() == false);
    //     imu.update().unwrap();
    // }
    // while(true){
    //     while(imu.is_mag_meas_done().unwrap() == false);
    //     imu.update().unwrap();
    //     const auto mag = imu.read_mag().unwrap() - base;

    //     calibrator.add_data(mag);
    //     const auto prog = calibrator.get_progress();
    //     DEBUG_PRINTLN(prog, mag);
    //     if(calibrator.get_percentage() >= 48){
    //         // const auto [mean, soft_iron] = calibrate_magfield(data);
    //         std::tie(mean, soft_iron) = calibrator.get_result();
    //         break;
    //     }
    // }

    for(size_t i = 0; i < 100; i++){
        const auto v2 = project_idx_to_v2(i, 100);
        // const auto p = project_v2_to_v3();
        const auto p = Vector3<q24>();
        DEBUG_PRINTLN(p,i, v2);
        clock::delay(1ms);
        calibrator.add_data(p + Vector3<q24>(0.2_r,1,1));
    }
    std::tie(mean, soft_iron) = calibrator.get_result();
    // DEBUG_PRINTLN(mean, soft_iron);
    PANIC(mean, soft_iron);
    for(const auto & mag : calibrator.data()){
        DEBUG_PRINTLN(mag);
        clock::delay(1ms);
    }
    while(true){
        while(imu.is_mag_meas_done().unwrap() == false);
        imu.update().unwrap();
        const auto mag = imu.read_mag().unwrap() - base;

        DEBUG_PRINTLN((mag - mean) * soft_iron);
    }
    // imu.set_mode(MMC5983::Mode::Cont4).unwrap();
    // DEBUG_PRINTLN("app started");

    // Quat<q24> gest;
    // auto measure = [&](){
    //     imu.update().unwrap();
    //     const auto dir = imu.read_mag().unwrap();
    //     gest = gest.slerp(Quat<q24>::from_direction(dir), 0.05_r);
    // };

    // timer1.init(FS);
    // timer1.attach(TimerIT::Update, {0,0},[&]{
    //     measure();
    // });
    
    // while(true){
    //     // DEBUG_PRINTLN(imu.update());
    //     DEBUG_PRINTLN(millis(), gest, imu.read_mag());
    // }
}

void mmc5983_main(){
    UART.init({576_KHz});
    DEBUGGER.retarget(&UART);
    DEBUGGER.no_brackets();
    DEBUGGER.set_eps(4);
    DEBUGGER.force_sync(EN);

    // I2cSw i2c{portA[12], portA[15]};
    I2cSw i2c{&SCL_GPIO, &SDA_GPIO};
    // i2c.init(400_KHz);
    i2c.init(200_KHz);
    // i2c.init();

    clock::delay(200ms);


    MMC5983 imu{&i2c};

    // auto & spi = spi1;
    // spi.init(18_MHz);
    // MMC5983 imu = {SpiDrv(spi, spi.attach_next_cs(portA[15]).value())};

    mmc5983_test(imu);
}