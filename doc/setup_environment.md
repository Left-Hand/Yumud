
## 🔨开发环境

使用C++20编写，请使用gcc12及以上的版本进行编译，否则可能出现编译错误
使用Vscode下的eide搭建开发环境，计划将于项目成熟后逐步迁移到Cmake中

由于厂方的sdk库已改动，请使用用本仓库提供的sdk 位于sdk目录下
#### GCC
由于本项目使用GCC进行开发（对各平台实现最大化开源），且目前暂无计划打算增加对Clang系列编译器的支持（ARMCC/ARMCLANG不支持）
且本项目使用到了完整的C++20的特性进行开发，故GCC的版本不低于12

#### CH32系列单片机开发：

同时由于CH32系列单片机使用的RiscV V4系列内部为了支持快速中断，为GCC添加了客制化的中断关键字，故应使用WCH定制的GCC12（需要安装[MounRiver Studio](http://www.mounriver.com/)以获取我们所需要的厂家客制化的riscv-none-elf-gcc）

#### VSCODE 
为了提供插件EIDE的宿主以及获得更好的编码体验 需要下载vscode并安装

#### EIDE

为了提供方便快捷开箱即用的环境体验 本项目选择vscode的插件eide进行开发 具体请移步
[eide](https://em-ide.com/zh-cn/docs/intro/)

#### OPENOCD

为了支持WCH-Link基于OPENOCD进行了定制化开发故需要安装WCH提供的OpenOCD(同样在MounRiver Studio的安装目录中可以找到)

#### 编译选项
请使用Os/O3/Ofast进行编译，同时开启lto优化，以确保代码的尺寸和运行速度被尽可能优化, 否则可能导致零成本抽象部分无法完全展开，FLASH占用过高无法使用。
