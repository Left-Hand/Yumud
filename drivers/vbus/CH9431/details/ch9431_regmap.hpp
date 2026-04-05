#pragma once
    
    // 00h R8_RXF0SIDL 接收过滤寄存器0的标准标识符低位  
    // 01h R8_RXF0SIDH 接收过滤寄存器0的标准标识符高位  
    // 02h R8_RXF0EIDL 接收过滤寄存器0的扩展标识符低位  
    // 03h R8_RXF0EIDH 接收过滤寄存器0的扩展标识符高位  
    // 04h R8_RXF1SIDL 接收过滤寄存器1的标准标识符低位  
    // 05h R8_RXF1SIDH 接收过滤寄存器1的标准标识符高位  
    // 06h R8_RXF1EIDL 接收过滤寄存器1的扩展标识符低位  
    // 07h R8_RXF1EIDH 接收过滤寄存器1的扩展标识符高位  
    // 08h R8_RXF2SIDL 接收过滤寄存器2的标准标识符低位  
    // 09h R8_RXF2SIDH 接收过滤寄存器2的标准标识符高位  
    // 0Ah R8_RXF2EIDL 接收过滤寄存器2的扩展标识符低位  
    // 0Bh R8_RXF2EIDH 接收过滤寄存器2的扩展标识符高位  
    // 0Ch R8_RXIPCTRL RXnBF引脚控制寄存器和状态寄存器 Y 
    // 0Dh R8_TXRTSCTRL TXnRTS引脚控制和状态寄存器 Y 
    // 0Eh R8_SYSSTAT 系统状态寄存器  
    // 0Fh R8_SYSCTRL 系统控制寄存器 Y 
    // 10h R8_RXF3SIDL 接收过滤寄存器3的标准标识符低位  
    // 11h R8_RXF3SIDH 接收过滤寄存器3的标准标识符高位  
    // 12h R8_RXF3EIDL 接收过滤寄存器3的扩展标识符低位  
    // 13h R8_RXF3EIDH 接收过滤寄存器3的扩展标识符高位  
    // 14h R8_RXF4SIDL 接收过滤寄存器4的标准标识符低位  
    // 15h R8_RXF4SIDH 接收过滤寄存器4的标准标识符高位  
    // 16h R8_RXF4EIDL 接收过滤寄存器4的扩展标识符低位  
    // 17h R8_RXF4EIDH 接收过滤寄存器4的扩展标识符高位  
    // 18h R8_RXF5SIDL 接收过滤寄存器5的标准标识符低位  
    // 19h R8_RXF5SIDH 接收过滤寄存器5的标准标识符高位  
    // 1Ah R8_RXF5EIDL 接收过滤寄存器5的扩展标识符低位  
    // 1Bh R8_RXF5EIDH 接收过滤寄存器5的扩展标识符高位  
    // 1Ch R8_TEC 发送错误计数器  
    // 1Dh R8_REC 接收错误计数器  
    // 1Eh R8_SYSSTAT 系统状态寄存器  
    // 1Fh R8_SYSCTRL 系统控制寄存器 Y 
    // 20h R8_RXM0SIDL 接收屏蔽寄存器0的标准标识符低位  
    // 21h R8_RXM0SIDH 接收屏蔽寄存器0的标准标识符高位  
    // 22h R8_RXM0EIDL 接收屏蔽寄存器0的扩展标识符低位 
    // 23h R8_RXM0EIDH 接收屏蔽寄存器0的扩展标识符高位  
    // 24h R8_RXM1SIDL 接收屏蔽寄存器1的标准标识符低位  
    // 25h R8_RXM1SIDH 接收屏蔽寄存器1的标准标识符高位  
    // 26h R8_RXM1EIDL 接收屏蔽寄存器1的扩展标识符低位  
    // 27h R8_RXM1EIDH 接收屏蔽寄存器1的扩展标识符高位  
    // 28h R8_BTIMER3 配置寄存器3 Y 
    // 29h R8_BTIMER2 配置寄存器2 Y 
    // 2Ah R8_BTIMER1 配置寄存器1 Y 
    // 2Bh R8_SYSINTE 中断使能寄存器 Y 
    // 2Ch R8_SYSINTF 中断标志寄存器 Y 
    // 2Dh R8_EFLAG 错误标志寄存器 Y 
    // 2Eh R8_SYSSTAT 系统状态寄存器  
    // 2Fh R8_SYSCTRL 系统控制寄存器 Y 
    // 30h R8_TXB0CTRL 发送缓冲区0控制寄存器 Y 
    // 31h R8_TXB0SIDL 发送缓冲区0的标准标识符低位  
    // 32h R8_TXB0SIDH 发送缓冲区0的标准标识符高位  
    // 33h R8_TXB0EIDL 发送缓冲区0的扩展标识符低位  
    // 34h R8_TXB0EIDH 发送缓冲区0的扩展标识符高位  
    // 35h R8_TXB0DLC 发送缓冲区0长度寄存器  
    // 36h R8_TXB0D0 发送缓冲区0的数据字节0  
    // 37h R8_TXB0D1 发送缓冲区0的数据段字节1  
    // 38h R8_TXB0D2 发送缓冲区0的数据段字节2  
    // 39h R8_TXB0D3 发送缓冲区0的数据段字节3  
    // 3Ah R8_TXB0D4 发送缓冲区0的数据段字节4  
    // 3Bh R8_TXB0D5 发送缓冲区0的数据段字节5  
    // 3Ch R8_TXB0D6 发送缓冲区0的数据段字节6  
    // 3Dh R8_TXB0D7 发送缓冲区0的数据段字节7  
    // 3Eh R8_SYSSTAT 系统状态寄存器  
    // 3Fh R8_SYSCTRL 系统控制寄存器 Y 
    // 40h R8_TXB1CTRL 发送缓冲区1控制寄存器 Y 
    // 41h R8_TXB1SIDL 发送缓冲区1的标准标识符低位  
    // 42h R8_TXB1SIDH 发送缓冲区1的标准标识符高位  
    // 43h R8_TXB1EIDL 发送缓冲区1的扩展标识符低位  
    // 44h R8_TXB1EIDH 发送缓冲区1的扩展标识符高位  
    // 45h R8_TXB1DLC 发送缓冲区1长度寄存器  
    // 46h R8_TXB1D0 发送缓冲区1的数据段字节0  
    // 47h R8_TXB1D1 发送缓冲区1的数据段字节1  
    // 48h R8_TXB1D2 发送缓冲区1的数据段字节2  
    // 49h R8_TXB1D3 发送缓冲区1的数据段字节3  
    // 4Ah R8_TXB1D4 发送缓冲区1的数据段字节4  
    // 4Bh R8_TXB1D5 发送缓冲区1的数据段字节5  
    // 4Ch R8_TXB1D6 发送缓冲区1的数据段字节6  
    // 4Dh R8_TXB1D7 发送缓冲区1的数据段字节7  
    // 4Eh R8_SYSSTAT 系统状态寄存器  
    // 4Fh R8_SYSCTRL 系统控制寄存器 Y 
    // 50h R8_TXB2CTRL 发送缓冲区2控制寄存器 Y 
    // 51h R8_TXB2SIDL 发送缓冲区2的标准标识符低位  
    // 52h R8_TXB2SIDH 发送缓冲区2的标准标识符高位
    // 53h R8_TXB2EIDL 发送缓冲区2的扩展标识符低位  
    // 54h R8_TXB2EIDH 发送缓冲区2的扩展标识符高位  
    // 55h R8_TXB2DLC 发送缓冲区2长度寄存器  
    // 56h R8_TXB2D0 发送缓冲区2的数据段字节0  
    // 57h R8_TXB2D1 发送缓冲区2的数据段字节1  
    // 58h R8_TXB2D2 发送缓冲区2的数据段字节2  
    // 59h R8_TXB2D3 发送缓冲区2的数据段字节3  
    // 5Ah R8_TXB2D4 发送缓冲区2的数据段字节4  
    // 5Bh R8_TXB2D5 发送缓冲区2的数据段字节5  
    // 5Ch R8_TXB2D6 发送缓冲区2的数据段字节6  
    // 5Dh R8_TXB2D7 发送缓冲区2的数据段字节7  
    // 5Eh R8_SYSSTAT 系统状态寄存器  
    // 5Fh R8_SYSCTRL 系统控制寄存器 Y 
    // 60h R8_RXB0CTRL 接收缓冲区0控制寄存器 Y 
    // 61h R8_RXB0SIDL 接收寄存器0的标准标识符低位  
    // 62h R8_RXB0SIDH 接收寄存器0的标准标识符高位  
    // 63h R8_RXB0EIDL 接收寄存器0的扩展标识符低位  
    // 64h R8_RXB0EIDH 接收寄存器0的扩展标识符高位  
    // 65h R8_RXB0DLC 接收缓冲区0长度寄存器  
    // 66h R8_RXB0D0 接收缓冲区0的数据段字节0  
    // 67h R8_RXB0D1 接收缓冲区0的数据段字节1  
    // 68h R8_RXB0D2 接收缓冲区0的数据段字节2  
    // 69h R8_RXB0D3 接收缓冲区0的数据段字节3  
    // 6Ah R8_RXB0D4 接收缓冲区0的数据段字节4  
    // 6Bh R8_RXB0D5 接收缓冲区0的数据段字节5  
    // 6Ch R8_RXB0D6 接收缓冲区0的数据段字节6  
    // 6Dh R8_RXB0D7 接收缓冲区0的数据段字节7  
    // 6Eh R8_SYSSTAT 系统状态寄存器  
    // 6Fh R8_SYSCTRL 系统控制寄存器 Y 
    // 70h R8_RXB1CTRL 接收缓冲区1控制寄存器  
    // 71h R8_RXB1SIDL 接收寄存器1的标准标识符低位  
    // 72h R8_RXB1SIDH 接收寄存器1的标准标识符高位  
    // 73h R8_RXB1EIDL 接收寄存器1的扩展标识符低位  
    // 74h R8_RXB1EIDH 接收寄存器1的扩展标识符高位  
    // 75h R8_RXB1DLC 接收缓冲区1长度寄存器  
    // 76h R8_RXB1D0 接收缓冲区1的数据段字节0  
    // 77h R8_RXB1D1 接收缓冲区1的数据段字节1  
    // 78h R8_RXB1D2 接收缓冲区1的数据段字节2  
    // 79h R8_RXB1D3 接收缓冲区1的数据段字节3  
    // 7Ah R8_RXB1D4 接收缓冲区1的数据段字节4  
    // 7Bh R8_RXB1D5 接收缓冲区1的数据段字节5  
    // 7Ch R8_RXB1D6 接收缓冲区1的数据段字节6  
    // 7Dh R8_RXB1D7 接收缓冲区1的数据段字节7  
    // 7Eh R8_SYSSTAT 系统状态寄存器  
    // 7Fh R8_SYSCTRL 系统控制寄存器 