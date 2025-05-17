#include "YT8512C.hpp"

using namespace ymd::drivers;

/* PHY芯片寄存器映射表 */ 
#define YT8512C_BCR                            ((uint16_t)0x0000U)
#define YT8512C_BSR                            ((uint16_t)0x0001U)
#define PHY_REGISTER2                           ((uint16_t)0x0002U)
#define PHY_REGISTER3                           ((uint16_t)0x0003U)

/* 操作SCR寄存器的值（一般不需要修改） */
#define YT8512C_BCR_SOFT_RESET                 ((uint16_t)0x8000U)
#define YT8512C_BCR_LOOPBACK                   ((uint16_t)0x4000U)
#define YT8512C_BCR_SPEED_SELECT               ((uint16_t)0x2000U)
#define YT8512C_BCR_AUTONEGO_EN                ((uint16_t)0x1000U)
#define YT8512C_BCR_POWER_DOWN                 ((uint16_t)0x0800U)
#define YT8512C_BCR_ISOLATE                    ((uint16_t)0x0400U)
#define YT8512C_BCR_RESTART_AUTONEGO           ((uint16_t)0x0200U)
#define YT8512C_BCR_DUPLEX_MODE                ((uint16_t)0x0100U) 

/* 操作BSR寄存器的值（一般不需要修改） */   
#define YT8512C_BSR_100BASE_T4                 ((uint16_t)0x8000U)
#define YT8512C_BSR_100BASE_TX_FD              ((uint16_t)0x4000U)
#define YT8512C_BSR_100BASE_TX_HD              ((uint16_t)0x2000U)
#define YT8512C_BSR_10BASE_T_FD                ((uint16_t)0x1000U)
#define YT8512C_BSR_10BASE_T_HD                ((uint16_t)0x0800U)
#define YT8512C_BSR_100BASE_T2_FD              ((uint16_t)0x0400U)
#define YT8512C_BSR_100BASE_T2_HD              ((uint16_t)0x0200U)
#define YT8512C_BSR_EXTENDED_STATUS            ((uint16_t)0x0100U)
#define YT8512C_BSR_AUTONEGO_CPLT              ((uint16_t)0x0020U)
#define YT8512C_BSR_REMOTE_FAULT               ((uint16_t)0x0010U)
#define YT8512C_BSR_AUTONEGO_ABILITY           ((uint16_t)0x0008U)
#define YT8512C_BSR_LINK_STATUS                ((uint16_t)0x0004U)
#define YT8512C_BSR_JABBER_DETECT              ((uint16_t)0x0002U)
#define YT8512C_BSR_EXTENDED_CAP               ((uint16_t)0x0001U)

/* PHY芯片进程状态 */
#define  YT8512C_STATUS_READ_ERROR             ((int32_t)-5)
#define  YT8512C_STATUS_WRITE_ERROR            ((int32_t)-4)
#define  YT8512C_STATUS_ADDRESS_ERROR          ((int32_t)-3)
#define  YT8512C_STATUS_RESET_TIMEOUT          ((int32_t)-2)
#define  YT8512C_STATUS_ERROR                  ((int32_t)-1)
#define  YT8512C_STATUS_OK                     ((int32_t) 0)
#define  YT8512C_STATUS_LINK_DOWN              ((int32_t) 1)
#define  YT8512C_STATUS_100MBITS_FULLDUPLEX    ((int32_t) 2)
#define  YT8512C_STATUS_100MBITS_HALFDUPLEX    ((int32_t) 3)
#define  YT8512C_STATUS_10MBITS_FULLDUPLEX     ((int32_t) 4)
#define  YT8512C_STATUS_10MBITS_HALFDUPLEX     ((int32_t) 5)
#define  YT8512C_STATUS_AUTONEGO_NOTDONE       ((int32_t) 6)

/* PHY地址 ---- 由用户设置 */
#define YT8512C_ADDR                           ((uint16_t)0x0000U)
/* PHY寄存器的数量 */
#define YT8512C_PHY_COUNT                      ((uint16_t)0x001FU)

#define YT8512C_PHYSCSR                        ((uint16_t)0x11)                       /*!< tranceiver status register */
#define YT8512C_SPEED_STATUS                   ((uint16_t)0x4010)                     /*!< configured information of speed: 100Mbit/s */
#define YT8512C_DUPLEX_STATUS                  ((uint16_t)0x2000)                     /*!< configured information of duplex: full-duplex */


#define YT8512C_SW_RESET_TO    ((uint32_t)500U)    /* 软件复位等待时间 */
#define YT8512C_INIT_TO        ((uint32_t)2000U)   /* 初始化等待时间 */
#define YT8512C_MAX_DEV_ADDR   ((uint32_t)31U)     /* PHY地址的最大值 */

#define YT8512C_AND_RTL8201BL_PHYREGISTER2      0x0000

/**
  * @brief       初始化YT8512C并配置所需的硬件资源
  * @param       pobj: 设备对象
  * @retval      YT8512C_STATUS_OK：初始化YT8512C并配置所需的硬件资源成功
                 YT8512C_STATUS_ADDRESS_ERROR：找不到设备地址
                 YT8512C_STATUS_READ_ERROR：不能读取寄存器
                 YT8512C_STATUS_WRITE_ERROR：不能写入寄存器
                 YT8512C_STATUS_RESET_TIMEOUT：无法执行软件复位
  */
int32_t YT8512C::init()
{
    uint32_t tickstart = 0, regvalue = 0, addr = 0;
    int32_t status = YT8512C_STATUS_OK;
    

    /* 设置PHY地址为32 */
    this->devaddr = YT8512C_MAX_DEV_ADDR + 1;

    /* 主要为了查找PHY地址 */  
    for (addr = 0; addr <= YT8512C_MAX_DEV_ADDR; addr ++)
    {
        if (this->readreg(addr, YT8512C_PHYSCSR, &regvalue) < 0)
        { 
            status = YT8512C_STATUS_READ_ERROR;
            /* 无法读取这个设备地址继续下一个地址 */
            continue;
        }
        /* 已经找到PHY地址了 */
        if ((regvalue & YT8512C_PHY_COUNT) == addr)
        {
            this->devaddr = addr;
            status = YT8512C_STATUS_OK;
            break;
        }
    }
    
    /* 判断这个PHY地址是否大于32（2^5）*/
    if (this->devaddr > YT8512C_MAX_DEV_ADDR)
    {
        status = YT8512C_STATUS_ADDRESS_ERROR;
    }

    /* 如果PHY地址有效 */
    if (status == YT8512C_STATUS_OK)
    {
        /* 设置软件复位  */
        if (this->writereg(this->devaddr, YT8512C_BCR, YT8512C_BCR_SOFT_RESET) >= 0)
        { 
            /* 获取软件重置状态 */
            if (this->readreg(this->devaddr, YT8512C_BCR, &regvalue) >= 0)
            { 
                tickstart = millis();

                /* 等待软件复位完成或超时  */
                while (regvalue & YT8512C_BCR_SOFT_RESET)
                {
                    if ((millis() - tickstart) <= YT8512C_SW_RESET_TO)
                    {
                        if (this->readreg(this->devaddr, YT8512C_BCR, &regvalue) < 0)
                        { 
                            status = YT8512C_STATUS_READ_ERROR;
                            break;
                        }
                    }
                    else
                    {
                        status = YT8512C_STATUS_RESET_TIMEOUT;
                        break;
                    }
                } 
            }
            else
            {
                status = YT8512C_STATUS_READ_ERROR;
            }
        }
        else
        {
            status = YT8512C_STATUS_WRITE_ERROR;
        }
    }
    
    /* 到了这里，初始化完成！！！ */
    if (status == YT8512C_STATUS_OK){
        /* 等待2s进行初始化 */
        clock::delay(YT8512C_INIT_TOms);
    }

    return status;
}



/**
  * @brief       反初始化YT8512C及其硬件资源
  * @param       pobj: 设备对象
  * @retval      YT8512C_STATUS_OK：反初始化失败成功
                 YT8512C_STATUS_ERROR：反初始化失败
  */
int32_t YT8512C::deinit(){

    return YT8512C_STATUS_OK;
}


/**
  * @brief       关闭YT8512C的下电模式
  * @param       pobj: 设备对象
  * @retval      YT8512C_STATUS_OK：关闭成功
                 YT8512C_STATUS_READ_ERROR：不能读取寄存器
                 YT8512C_STATUS_WRITE_ERROR：不能写寄存器
  */
int32_t YT8512C::disable_power_down_mode()
{
    uint32_t readval = 0;
    int32_t status = YT8512C_STATUS_OK;

    if (this->readreg(this->devaddr, YT8512C_BCR, &readval) >= 0)
    {
        readval &= ~YT8512C_BCR_POWER_DOWN;

        /* 清除下电模式 */
        if (this->writereg(this->devaddr, YT8512C_BCR, readval) < 0)
        {
            status =  YT8512C_STATUS_WRITE_ERROR;
        }
    }
    else
    {
        status = YT8512C_STATUS_READ_ERROR;
    }

    return status;
}



/**
  * @brief       使能YT8512C的下电模式
  * @param       pobj: 设备对象
  * @retval      YT8512C_STATUS_OK：关闭成功
                 YT8512C_STATUS_READ_ERROR：不能读取寄存器
                 YT8512C_STATUS_WRITE_ERROR：不能写寄存器
  */
int32_t YT8512C::enable_power_down_mode()
{
    uint32_t readval = 0;
    int32_t status = YT8512C_STATUS_OK;

    if (this->readreg(this->devaddr, YT8512C_BCR, &readval) >= 0)
    {
        readval |= YT8512C_BCR_POWER_DOWN;

        /* 使能下电模式 */
        if (this->writereg(this->devaddr, YT8512C_BCR, readval) < 0)
        {
            status =  YT8512C_STATUS_WRITE_ERROR;
        }
    }
    else
    {
        status = YT8512C_STATUS_READ_ERROR;
    }

    return status;
}



/**
  * @brief       启动自动协商过程
  * @param       pobj: 设备对象
  * @retval      YT8512C_STATUS_OK：关闭成功
                 YT8512C_STATUS_READ_ERROR：不能读取寄存器
                 YT8512C_STATUS_WRITE_ERROR：不能写寄存器
  */
int32_t YT8512C::start_auto_nego()
{
    uint32_t readval = 0;
    int32_t status = YT8512C_STATUS_OK;

    if (this->readreg(this->devaddr, YT8512C_BCR, &readval) >= 0)
    {
        readval |= YT8512C_BCR_AUTONEGO_EN;

        /* 启动自动协商 */
        if (this->writereg(this->devaddr, YT8512C_BCR, readval) < 0)
        {
            status =  YT8512C_STATUS_WRITE_ERROR;
        }
    }
    else
    {
        status = YT8512C_STATUS_READ_ERROR;
    }

    return status;
}



/**
  * @brief       获取YT8512C设备的链路状态
  * @param       pobj: 设备对象
  * @param       pLinkState: 指向链路状态的指针
  * @retval      YT8512C_STATUS_100MBITS_FULLDUPLEX：100M，全双工
                 YT8512C_STATUS_100MBITS_HALFDUPLEX ：100M，半双工
                 YT8512C_STATUS_10MBITS_FULLDUPLEX：10M，全双工
                 YT8512C_STATUS_10MBITS_HALFDUPLEX ：10M，半双工
                 YT8512C_STATUS_READ_ERROR：不能读取寄存器
  */
int32_t YT8512C::get_link_state()
{
    uint32_t readval = 0;

    /* 检测特殊功能寄存器链接值 */
    if (this->readreg(this->devaddr, YT8512C_PHYSCSR, &readval) < 0)
    {
        return YT8512C_STATUS_READ_ERROR;
    }

    if (((readval & YT8512C_SPEED_STATUS) != YT8512C_SPEED_STATUS) && ((readval & YT8512C_DUPLEX_STATUS) != 0))
    {
        return YT8512C_STATUS_100MBITS_FULLDUPLEX;
    }
    else if (((readval & YT8512C_SPEED_STATUS) != YT8512C_SPEED_STATUS))
    {
        return YT8512C_STATUS_100MBITS_HALFDUPLEX;
    }
    else if (((readval & YT8512C_BCR_DUPLEX_MODE) != YT8512C_BCR_DUPLEX_MODE))
    {
        return YT8512C_STATUS_10MBITS_FULLDUPLEX;
    }
    else
    {
        return YT8512C_STATUS_10MBITS_HALFDUPLEX;
    }
}


/**
  * @brief       设置YT8512C设备的链路状态
  * @param       pobj: 设备对象
  * @param       pLinkState: 指向链路状态的指针
  * @retval      YT8512C_STATUS_OK：设置成功
                 YT8512C_STATUS_ERROR ：设置失败
                 YT8512C_STATUS_READ_ERROR：不能读取寄存器
                 YT8512C_STATUS_WRITE_ERROR ：不能写入寄存器
  */
int32_t YT8512C::set_link_state(uint32_t linkstate)
{
    uint32_t bcrvalue = 0;
    int32_t status = YT8512C_STATUS_OK;

    if (this->readreg(this->devaddr, YT8512C_BCR, &bcrvalue) >= 0)
    {
        /* 禁用链路配置(自动协商，速度和双工) */
        bcrvalue &= ~(YT8512C_BCR_AUTONEGO_EN | YT8512C_BCR_SPEED_SELECT | YT8512C_BCR_DUPLEX_MODE);

        if (linkstate == YT8512C_STATUS_100MBITS_FULLDUPLEX)
        {
            bcrvalue |= (YT8512C_BCR_SPEED_SELECT | YT8512C_BCR_DUPLEX_MODE);
        }
        else if (linkstate == YT8512C_STATUS_100MBITS_HALFDUPLEX)
        {
            bcrvalue |= YT8512C_BCR_SPEED_SELECT;
        }
        else if (linkstate == YT8512C_STATUS_10MBITS_FULLDUPLEX)
        {
            bcrvalue |= YT8512C_BCR_DUPLEX_MODE;
        }
        else
        {
            /* 错误的链路状态参数 */
            status = YT8512C_STATUS_ERROR;
        }
    }
    else
    {
        status = YT8512C_STATUS_READ_ERROR;
    }

    if(status == YT8512C_STATUS_OK)
    {
        /* 写入链路状态 */
        if(this->writereg(this->devaddr, YT8512C_BCR, bcrvalue) < 0)
        {
            status = YT8512C_STATUS_WRITE_ERROR;
        }
    }

    return status;
}

/**
  * @brief       启用环回模式
  * @param       pobj: 设备对象
  * @param       pLinkState: 指向链路状态的指针
  * @retval      YT8512C_STATUS_OK：设置成功
                 YT8512C_STATUS_READ_ERROR：不能读取寄存器
                 YT8512C_STATUS_WRITE_ERROR ：不能写入寄存器
  */
int32_t YT8512C::enable_loop_back_mode()
{
    uint32_t readval = 0;
    int32_t status = YT8512C_STATUS_OK;

    if (this->readreg(this->devaddr, YT8512C_BCR, &readval) >= 0)
    {
        readval |= YT8512C_BCR_LOOPBACK;

        /* 启用环回模式 */
        if (this->writereg(this->devaddr, YT8512C_BCR, readval) < 0)
        {
            status = YT8512C_STATUS_WRITE_ERROR;
        }
    }
    else
    {
        status = YT8512C_STATUS_READ_ERROR;
    }

    return status;
}

/**
  * @brief       禁用环回模式
  * @param       pobj: 设备对象
  * @param       pLinkState: 指向链路状态的指针
  * @retval      YT8512C_STATUS_OK：设置成功
                 YT8512C_STATUS_READ_ERROR：不能读取寄存器
                 YT8512C_STATUS_WRITE_ERROR ：不能写入寄存器
  */
int32_t YT8512C::disable_loop_back_mode()
{
    uint32_t readval = 0;
    int32_t status = YT8512C_STATUS_OK;

    if (this->readreg(this->devaddr, YT8512C_BCR, &readval) >= 0)
    {
        readval &= ~YT8512C_BCR_LOOPBACK;

        /* 禁用环回模式 */
        if (this->writereg(this->devaddr, YT8512C_BCR, readval) < 0)
        {
            status =  YT8512C_STATUS_WRITE_ERROR;
        }
    }
    else
    {
        status = YT8512C_STATUS_READ_ERROR;
    }

    return status;
}

