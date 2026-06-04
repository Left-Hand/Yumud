#pragma once

#include "damiao_primitive.hpp"

namespace ymd::robots::damiao{

namespace details{
template<Package K>
struct LimitParamTable;

#define DEF_DAMIAO_MOTOR_LIMIT_TABLE(K, pMax, vMax, tMax) \
template<> \
struct LimitParamTable<K> { \
    static constexpr LimitParam table = {static_cast<float>(pMax), static_cast<float>(vMax), static_cast<float>(tMax)}; \
};

DEF_DAMIAO_MOTOR_LIMIT_TABLE(Package::DM3507,         12.5, 50, 5)
DEF_DAMIAO_MOTOR_LIMIT_TABLE(Package::DM4310,         12.5, 30, 10) 
DEF_DAMIAO_MOTOR_LIMIT_TABLE(Package::DM4310_48V,     12.5, 50, 10) 
DEF_DAMIAO_MOTOR_LIMIT_TABLE(Package::DM4340,         12.5, 8, 28)  
DEF_DAMIAO_MOTOR_LIMIT_TABLE(Package::DM4340_48V,     12.5, 10, 28) 
DEF_DAMIAO_MOTOR_LIMIT_TABLE(Package::DM6006,         12.5, 45, 20) 
DEF_DAMIAO_MOTOR_LIMIT_TABLE(Package::DM8006,         12.5, 45, 40) 
DEF_DAMIAO_MOTOR_LIMIT_TABLE(Package::DM8009,         12.5, 45, 54) 
DEF_DAMIAO_MOTOR_LIMIT_TABLE(Package::DM10010L,       12.5, 25, 200)
DEF_DAMIAO_MOTOR_LIMIT_TABLE(Package::DM10010,        12.5, 20, 200)
DEF_DAMIAO_MOTOR_LIMIT_TABLE(Package::DMH3510,        12.5, 280, 1) 
DEF_DAMIAO_MOTOR_LIMIT_TABLE(Package::DMH6215,        12.5, 45, 10) 
DEF_DAMIAO_MOTOR_LIMIT_TABLE(Package::DMG6220,        12.5, 45, 10) 

#undef DEF_DAMIAO_MOTOR_LIMIT_TABLE
}



}