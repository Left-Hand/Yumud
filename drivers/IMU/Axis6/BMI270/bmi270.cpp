#include "bmi270.hpp"

void BMI270::init(){
    uint8_t return_state = 0;
    // readReg(IMU660RA_CHIP_ID);                                   // 读取一下设备ID 将设备设置为SPI模式
    do{
        if(getChipId() != valid_chipid)                                               // IMU660RA 自检
        {
            // 如果程序在输出了断言信息 并且提示出错位置在这里
            // 那么就是 IMU660RA 自检出错并超时退出了
            // 检查一下接线有没有问题 如果没问题可能就是坏了
            BMI270_DEBUG("imu660ra self check error.");
            return_state = 1;
            break;
        }
        writeReg(IMU660RA_PWR_CONF, 0x00);                       // 关闭高级省电模式
        delay(1);
        writeReg(IMU660RA_INIT_CTRL, 0x00);                      // 开始对模块进行初始化配置
        writeRegs(IMU660RA_INIT_DATA, bmi270_config_file, sizeof(bmi270_config_file));   // 输出配置文件
        writeReg(IMU660RA_INIT_CTRL, 0x01);                      // 初始化配置结束
        delay(20);
        uint8_t ret = 0;
        readReg(IMU660RA_INT_STA, ret);
        if(ret)                       // 检查是否配置完成
        {
            // 如果程序在输出了断言信息 并且提示出错位置在这里
            // 那么就是 IMU660RA 配置初始化文件出错了
            // 检查一下接线有没有问题 如果没问题可能就是坏了
            BMI270_DEBUG("imu660ra init error.");
            return_state = 1;
            break;
        }
        writeReg(IMU660RA_PWR_CTRL, 0x0E);                       // 开启性能模式  使能陀螺仪、加速度、温度传感器
        writeReg(IMU660RA_ACC_CONF, 0xA7);                       // 加速度采集配置 性能模式 正常采集 50Hz  采样频率
        writeReg(IMU660RA_GYR_CONF, 0xA9);                       // 陀螺仪采集配置 性能模式 正常采集 200Hz 采样频率

        // IMU660RA_ACC_SAMPLE 寄存器
        // 设置为 0x00 加速度计量程为 ±2  g   获取到的加速度计数据除以 16384  可以转化为带物理单位的数据 单位 g(m/s^2)
        // 设置为 0x01 加速度计量程为 ±4  g   获取到的加速度计数据除以 8192   可以转化为带物理单位的数据 单位 g(m/s^2)
        // 设置为 0x02 加速度计量程为 ±8  g   获取到的加速度计数据除以 4096   可以转化为带物理单位的数据 单位 g(m/s^2)
        // 设置为 0x03 加速度计量程为 ±16 g   获取到的加速度计数据除以 2048   可以转化为带物理单位的数据 单位 g(m/s^2)
        switch(IMU660RA_ACC_SAMPLE_DEFAULT)
        {
            default:
            {
                BMI270_DEBUG("IMU660RA_ACC_SAMPLE_DEFAULT set error.");
                return_state = 1;
            }break;
            case IMU660RA_ACC_SAMPLE_SGN_2G:
            {
                writeReg(IMU660RA_ACC_RANGE, 0x00);
                accel_scaler = (real_t)16384;
            }break;
            case IMU660RA_ACC_SAMPLE_SGN_4G:
            {
                writeReg(IMU660RA_ACC_RANGE, 0x01);
                accel_scaler = (real_t)8192;
            }break;
            case IMU660RA_ACC_SAMPLE_SGN_8G:
            {
                writeReg(IMU660RA_ACC_RANGE, 0x02);
                accel_scaler = (real_t)4096;
            }break;
            case IMU660RA_ACC_SAMPLE_SGN_16G:
            {
                writeReg(IMU660RA_ACC_RANGE, 0x03);
                accel_scaler = (real_t)2048;
            }break;
        }
        if(1 == return_state)
        {
            break;
        }

        // IMU660RA_GYR_RANGE 寄存器
        // 设置为 0x04 陀螺仪量程为 ±125  dps    获取到的陀螺仪数据除以 262.4   可以转化为带物理单位的数据 单位为 °/s
        // 设置为 0x03 陀螺仪量程为 ±250  dps    获取到的陀螺仪数据除以 131.2   可以转化为带物理单位的数据 单位为 °/s
        // 设置为 0x02 陀螺仪量程为 ±500  dps    获取到的陀螺仪数据除以 65.6    可以转化为带物理单位的数据 单位为 °/s
        // 设置为 0x01 陀螺仪量程为 ±1000 dps    获取到的陀螺仪数据除以 32.8    可以转化为带物理单位的数据 单位为 °/s
        // 设置为 0x00 陀螺仪量程为 ±2000 dps    获取到的陀螺仪数据除以 16.4    可以转化为带物理单位的数据 单位为 °/s
        switch(IMU660RA_GYRO_SAMPLE_DEFAULT)
        {
            default:
            {
                BMI270_DEBUG("IMU660RA_GYRO_SAMPLE_DEFAULT set error.");
                return_state = 1;
            }break;
            case IMU660RA_GYRO_SAMPLE_SGN_125DPS:
            {
                writeReg(IMU660RA_GYR_RANGE, 0x04);
                gyro_scaler = (real_t)262.4;
            }break;
            case IMU660RA_GYRO_SAMPLE_SGN_250DPS:
            {
                writeReg(IMU660RA_GYR_RANGE, 0x03);
                gyro_scaler = (real_t)131.2;
            }break;
            case IMU660RA_GYRO_SAMPLE_SGN_500DPS:
            {
                writeReg(IMU660RA_GYR_RANGE, 0x02);
                gyro_scaler = (real_t)65.6;
            }break;
            case IMU660RA_GYRO_SAMPLE_SGN_1000DPS:
            {
                writeReg(IMU660RA_GYR_RANGE, 0x01);
                gyro_scaler = (real_t)32.8;
            }break;
            case IMU660RA_GYRO_SAMPLE_SGN_2000DPS:
            {
                writeReg(IMU660RA_GYR_RANGE, 0x00);
                gyro_scaler = (real_t)16.4;
            }break;
        }
        if(1 == return_state)
        {
            break;
        }
    }while(0);

    // return return_state;
    // {
    //     uint8_t chipid = getChipId();
    //     if(chipid != valid_chipid){
    //         BMI270_DEBUG("BAD CHIP");
    //         return;
    //     }
    // }

    // writeReg(0x7c, 0x00);
    // delayMicroseconds(450);
    // writeReg(0x59, 0x00);
    // writeReg(0x59, 0x01);
    // writeReg(0x7d, 0x0e);
    // writeReg(0x40, 0xA8);
    // writeReg(0x42, 0xA9);
    // writeReg(0x7c, 0x02);

}