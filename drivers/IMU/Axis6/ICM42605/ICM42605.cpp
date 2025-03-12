#include "ICM42605.hpp"

#define ICM42605_DEVICE_CONFIG               (0x11)
#define ICM42605_DRIVE_CONFIG                (0x13)
#define ICM42605_ACC_DATA_X1               (0x1F)
#define ICM42605_ACC_DATA_X0               (0x20)
#define ICM42605_ACC_DATA_Y1               (0x21)
#define ICM42605_ACC_DATA_Y0               (0x22)
#define ICM42605_ACC_DATA_Z1               (0x23)
#define ICM42605_ACC_DATA_Z0               (0x24)
#define ICM42605_GYR_DATA_X1                (0x25)
#define ICM42605_GYR_DATA_X0                (0x26)
#define ICM42605_GYR_DATA_Y1                (0x27)
#define ICM42605_GYR_DATA_Y0                (0x28)
#define ICM42605_GYR_DATA_Z1                (0x29)
#define ICM42605_GYR_DATA_Z0                (0x2A)
#define ICM42605_TEMP_DATA1                  (0x1D)
#define ICM42605_TEMP_DATA0                  (0x1E)
#define ICM42605_PWR_MGMT0                   (0x4E)
#define ICM42605_GYR_CONFIG0                (0x4F)
#define ICM42605_ACC_CONFIG0               (0x50)
#define ICM42605_GYR_CONFIG1                (0x51)
#define ICM42605_GYR_ACC_CONFIG0          (0x52)
#define ICM42605_ACC_CONFIG1               (0x53)
#define ICM42605_WHO_AM_I                    (0x75)
#define ICM42605_BANK_SEL                    (0x76)
#define ICM42605_INTF_CONFIG4                (0x7A)

#define ICM_MODE_ACC                         (1<<0)
#define ICM_MODE_GYR                        (1<<1)
#define ICM_MODE_TEMP                        (1<<2)


using namespace ymd;
using namespace ymd::drivers;

void ICM42605::init() {
    writeReg(ICM42605_BANK_SEL, 0);
    writeReg(ICM42605_BANK_SEL, 1);
    writeReg(ICM42605_INTF_CONFIG4, 0x02);
    writeReg(ICM42605_BANK_SEL, 0);
    writeReg(ICM42605_GYR_CONFIG0, 0b00000110);
    writeReg(ICM42605_ACC_CONFIG0, 0b00000011);
    writeReg(ICM42605_PWR_MGMT0, 0b00011111);
}

bool ICM42605::verify(){
    writeReg(ICM42605_BANK_SEL, 0);
    uint8_t id = 0;
    readReg(ICM42605_WHO_AM_I, id);
    return (id == 0);//FIXME
}

// uint8_t ICM42605::GetData(icmData_t *icm, uint8_t MODE) {
//     int16_t out;
//     uint8_t data[6];

//     //加速度计
//     if (MODE & ICM_MODE_ACC) {
//         if (readRegs(ICM42605_ACC_DATA_X1, data, 6) == 0)
//             return 1;
//         out = (int16_t) (data[0] << 8 | data[1]);
//         icm->ax = (float) out * 16 / 32768.0f;
//         out = (int16_t) (data[2] << 8 | data[3]);
//         icm->ay = (float) out * 16 / 32768.0f;
//         out = (int16_t) (data[4] << 8 | data[5]);
//         icm->az = (float) out * 16 / 32768.0f;
//     }

//     //角速度计
//     if (MODE & ICM_MODE_GYR) {
//         if (readRegs(ICM42605_GYR_DATA_X1, data, 6) == 0)
//             return 2;
//         out = (int16_t) (data[0] << 8 | data[1]);
//         icm->gx = (float) out * 2000.0f / 32768.0f;
//         out = (int16_t) (data[2] << 8 | data[3]);
//         icm->gy = (float) out * 2000.0f / 32768.0f;
//         out = (int16_t) (data[4] << 8 | data[5]);
//         icm->gz = (float) out * 2000.0f / 32768.0f;
//     }

//     //温度计
//     if (MODE & ICM_MODE_TEMP) {
//         if (readRegs(ICM42605_TEMP_DATA0, data, 2) == 0)
//             return 3;
//         out = (int16_t) (data[0] << 8 | data[1]);
//         icm->temp = 25.0f + (float) out * 1.0f / 132.48f;
//     }

//     return 0;
// }


void ICM42605::update(){

}

void ICM42605::reset(){

}

Option<Vector3> ICM42605::getAcc(){
    return Some{Vector3{0,0,0}};//TODO
}

Option<Vector3> ICM42605::getGyr(){
    return Some{Vector3{0,0,0}};//TODO
}