#include <cstdint>

namespace ymd::robots::damiao::imul1{


// 00  重启IMU W
// 01  加速度数据R
// 02  角速度数据R
// 03  欧拉角数据R
// 04  四元数数据R
// 05  角度置零W
// 06  加计六面校准W
// 07  陀螺静态校准W
// 08  磁计椭球校准W
// 09  切换通信模式RW
// 0A  设置主动发送间隔RW
// 0B  切换主被动模式RW
// 0C  修改波特率RW
// 0D   CAN_ID RW
// 0E   MST_ID RW
// 0F  输出数据选择RW
// FE  保存参数W
// FF  恢复出厂设置W

enum class RegAddr:uint8_t{
    RestartImu = 0x00,
    AccData = 0x01,

};

enum class [[nodiscard]] AckErrc:uint8_t{
    Ok = 0x00,
    RegNonexist = 0x01,
    InvalidData = 0x02,
    OperateFailed = 0x03
};

}

