
1.阅读这份pdf文档 将寄存器字段输出为markdown文件 参考格式如下（在这个参考格式的基础上为每个寄存器附带偏移地址和复位值的属性）：

### 23.2.1.4 USB 杂项状态寄存器(R8_USB_MIS_ST)
| 位 | 名称 | 访问 | 描述 | 复位值 |
| ---- | ---- | ---- | ---- | ------ |
| 0 | RB_UMS_DEV_ATTACH | RO | USB 主机模式下端口的USB 设备连接状态位: 1:端口已经连接USB 设备; 0:端口没有USB 设备连接 | 0 |
| 1 | RB_UMS_DM_LEVEL | RO | USB 主机模式下，设备刚连入USB 端口时DM 引脚的电平状态，用于判断速度: 1:高电平/低速; 0:低电平/全速 | 0 |
| 2 | RB_UMS_SUSPEND | RO | USB 挂起状态位: 1:USB 总线处于挂起态，有一段时间没有USB 活动; 0:USB 总线处于非挂起态 | 0 |
| 3 | RB_UMS_BUS_RST | RO | USB 总线复位状态位: 1:当前USB 总线处于复位态; 0:当前USB 总线处于非复位态 | x |
| 4 | RB_UMS_R_FIFO_RDY | RO | USB 接收FIFO 数据就绪状态位: 1:接收FIFO 非空; 0:接收FIFO 为空 | 0 |
| 5 | RB_UMS_SIE_FREE | RO | USB 协议处理器的空闲状态位: 1:协议器空闲; 0:忙，正在进行USB 传输 | 1 |
| 6 | RB_UMS_SOF_ACT | RO | USB 主机模式下SOF 包传输状态位: 1:正在发出SOF 包; 0:发送完成或者空闲 | x |
| 7 | RB_UMS_SOF_PRES | RO | USB 主机模式下SOF 包预示状态位: 1:将要发送SOF 包，此时如有其它USB 数据包将被自动延后; 0:无SOF 包发送 | x |

2.参考这份代码将文档转换为位域 越低的比特放在每段描述的靠上的位置（如果描述足够简短把注释放在单行 否则放在成员前多行） 

``` cpp
//CAN配置和状态寄存器（8位）[0xA0] 复位值:0x80
struct [[nodiscard]] R8_CAN_CFG_STAT {
    uint8_t BUSOFF:1;       // 复位位                   复位值(0)
    uint8_t TACTIVE:1;      // 主发送缓冲区状态位       复位值(0)
    uint8_t RACTIVE:1;      // 次发送缓冲区状态位       复位值(0)
    uint8_t TSSS:1;         // 本地回环模式使能位       复位值(0)
    uint8_t TPSS:1;         // 本地回环模式中断标志位   复位值(0)
    uint8_t LBME:1;         // 接收激活位               复位值(0)
    uint8_t LBMI:1;         // 发送激活位               复位值(0)
    uint8_t RESET:1;        // 总线关闭位               复位值(1)
};
```

 生成每个寄存器的结构体 每个外设地址映射的结构体以及偏移测试代码