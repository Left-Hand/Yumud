#pragma once

// https://github.com/ch32-rs/wlink/blob/main/src/chips.rs

namespace ymd{

    enum class ChipName:uint32_t{
        // 基础型号
        CH565 = 0x65000000,
        CH569 = 0x69000000,
        CH571 = 0x71000000,
        CH573 = 0x73000000,
        CH581 = 0x81000000,
        CH582 = 0x82000000,
        CH583 = 0x83000000,
        CH584 = 0x84000000,
        CH592 = 0x92000000,
        CH585 = 0x93000000,

        // CH32V003 系列
        CH32V003F4P6 = 0x00300500,
        CH32V003F4U6 = 0x00310500,
        CH32V003A4M6 = 0x00320500,
        CH32V003J4M6 = 0x00330500,

        // CH32V006 系列
        CH32V006K8U6 = 0x00600600,
        CH32V006E8R6 = 0x00610600,
        CH32V006F8U6 = 0x00620600,
        CH32V006F8P6 = 0x00630600,

        // CH32V007 系列
        CH32M007G8R6 = 0x00700800,
        CH32V007E8R6 = 0x00710600,
        CH32V007F8U6 = 0x00720600,

        // CH32V005 系列
        CH32V005E6R6 = 0x00500600,
        CH32V005F6U6 = 0x00510600,
        CH32V005F6P6 = 0x00520600,
        CH32V005D6U6 = 0x00530600,

        // CH32V002 系列
        CH32V002F4P6 = 0x00200600,
        CH32V002F4U6 = 0x00210600,
        CH32V002A4M6 = 0x00220600,
        CH32V002D4U6 = 0x00230600,
        CH32V002J4M6 = 0x00240600,

        // CH32V004 系列
        CH32V004F6P1 = 0x00400600,
        CH32V004F6U1 = 0x00410600,

        // CH32X035 系列
        CH32X035R8T6 = 0x03500601,
        CH32X035C8T6 = 0x03510601,
        CH32X035F8U6 = 0x035E0601,
        CH32X035G8U6 = 0x03560601,
        CH32X035G8R6 = 0x035B0601,
        CH32X035F7P6 = 0x03570601,
        CH32X033F8P6 = 0x035A0601,

        // CH32L103 系列
        CH32L103C8U6 = 0x10300700,
        CH32L103C8T6 = 0x10310700,
        CH32L103F8P6 = 0x103A0700,
        CH32L103G8R6 = 0x103B0700,
        CH32L103K8U6 = 0x10320700,
        CH32L103F8U6 = 0x103D0700,
        CH32L103F7P6 = 0x10370700,

        // CH32F103 系列
        CH32F103C8T6 = 0x20004102,
        CH32F103R8T6 = 0x2000410F,

        // CH32V203 系列
        CH32V203C8U6 = 0x20300500,
        CH32V203C8T6 = 0x20310500,
        CH32V203K8T6 = 0x20320500,
        CH32V203C6T6 = 0x20330500,
        CH32V203K6T6 = 0x20350500,
        CH32V203G6U6 = 0x20360500,
        CH32V203G8R6 = 0x203B0500,
        CH32V203F8U6 = 0x203E0500,
        CH32V203F6P6 = 0x20370500,
        CH32V203RBT6 = 0x2034050C,

        // CH32V208 系列
        CH32V208WBU6 = 0x2080050C,
        CH32V208RBT6 = 0x2081050C,
        CH32V208CBU6 = 0x2082050C,
        CH32V208GBU6 = 0x2083050C,

        // CH32V303 系列
        CH32V303CBT6 = 0x30330504,
        CH32V303RBT6 = 0x30320504,
        CH32V303RCT6 = 0x30310504,
        CH32V303VCT6 = 0x30300504,

        // CH32V305 系列
        CH32V305FBP6 = 0x30520508,
        CH32V305RBT6 = 0x30500508,
        CH32V305GBU6 = 0x305B0508,

        // CH32V307 系列
        CH32V307WCU6 = 0x30730508,
        CH32V307FBP6 = 0x30720508,
        CH32V307RCT6 = 0x30710508,
        CH32V307VCT6 = 0x30700508,

        // CH32V317 系列
        CH32V317VCT6 = 0x3170B508,
        CH32V317WCU6 = 0x3173B508,
        CH32V317TCU6 = 0x3175B508,

        // 其他型号
        CH641F = 0x64100500,
        CH641D = 0x64110500,
        CH641U = 0x64150500,
        CH641P = 0x64160500,
        CH643W = 0x64300601,
        CH643Q = 0x64310601,
        CH643L = 0x64330601,
        CH643U = 0x64340601,

        UNKNOWN = 0xffffffffl
    };
}