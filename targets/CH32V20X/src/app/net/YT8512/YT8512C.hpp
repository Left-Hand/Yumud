#pragma once


#include "drivers/device_defs.h"

namespace ymd::drivers{


/* 注册到组件对象结构体 */
class YT8512C{
protected:
    uint32_t            devaddr;                        /* PHY地址 */
    uint32_t            is_initialized;                 /* 描述该设备是否初始化 */

    int32_t readreg(uint32_t, uint32_t, uint32_t *){
        //TODO
        return 0;
    }
    int32_t writereg(uint32_t, uint32_t, uint32_t){
        //TODO
        return 0;
    }

public:
    YT8512C();

    // int32_t regster_bus_io(yt8512c_ioc_tx_t *ioctx);             /* 将IO函数注册到组件对象 */
    int32_t init();                                                 /* 初始化YT8512C并配置所需的硬件资源 */
    int32_t deinit();                                               /* 反初始化YT8512C及其硬件资源 */
    int32_t disable_power_down_mode();                              /* 关闭YT8512C的下电模式 */
    int32_t enable_power_down_mode();                               /* 使能YT8512C的下电模式 */
    int32_t start_auto_nego();                                      /* 启动自动协商过程 */
    int32_t get_link_state();                                       /* 获取YT8512C设备的链路状态 */
    int32_t set_link_state(uint32_t linkstate);                   /* 设置YT8512C设备的链路状态 */
    int32_t enable_loop_back_mode();                                /* 启用环回模式 */
    int32_t disable_loop_back_mode();                               /* 禁用环回模式 */
    
};



}