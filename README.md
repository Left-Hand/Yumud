# Yumud

项目架构不稳定 作者不会用git 可能会强行推流 不要轻易fork！！！！！

算是用来练习git的项目

孩子自个写着玩的

用c写嵌入式太膈应了 面向对象多好

目前支持的芯片只有ch32(多好)

参考了HAL和arduino 但使用了抽象开销成本更小与更安全的方式编写

使用了大量cpp17+的特性 本来打算用cpp20写 但考虑到后续的移植问题(例如platformio下的esp32似乎只支持到cpp17) 还是用17写

再说20的语法糖多少也太炫了 一方面没那个能力 以方面没那个强烈需求

使用eide搭建开发环境

## SDK
厂方sdk库已改动 不是原本 用本仓库提供的sdk
iqmath选择16位

## ThirdParty
不会submodule 直接拷本地了(开摆)

## Build
gcc12+(wch特供版)
os/o3
cpp >= cpp17