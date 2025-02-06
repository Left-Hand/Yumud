#pragma once

#include "utils.hpp"
#include "PdoProtocol.hpp"

namespace ymd::canopen{



class PdoSession {
public:
    using Driver = CanDriver;
    using CobId = uint16_t;


    // 每个 PDO 都有一个通信参数对象，用于配置 PDO 的通信行为。

    // 子索引：

    //  0x00：支持的数量。
    //  0x01：COB-ID（Communication Object Identifier），用于标识 PDO 的 CAN 帧 ID。
    //  0x02：传输类型（Transmission Type），定义 PDO 的触发方式（如事件触发、定时触发等）。
    //  0x03：禁止时间（Inhibit Time），定义两次 PDO 传输之间的最小时间间隔。
    //  0x04：事件定时器（Event Timer），定义 PDO 的定时触发时间。
    //  0x05：同步起始值（SYNC Start Value），用于同步 PDO 传输。
    class ParamsSubIndex{
        enum _Enum:uint8_t{
            NUM,
            COBID,
            TRANSMISSION_TYPE,
            INHIBIT_TIME,
            EVENT_TIMER,
            SYNC_START_VALUE
        };

        ParamsSubIndex(const _Enum e):e_(e){;}
        operator OdSubIndex() const {
            return static_cast<OdSubIndex>(e_);
        }

        private:
            _Enum e_;
    };

    // 为0时表示，映射数据变化并且收到一个同步帧，才会发送TPDO。
    // 为1~240时表示，收到相应个数的同步帧时就发送PDO，和映射数据是否变化没有关系。
    // 为254、255时表示，映射数据改变或事件计时器到，就会发送PDO。 我这里测试为254或者255的情况下，如果映射数据改变的时候，不会发送PDO，只有事件计时器到才会发送PDO
    // 对于RPDO而言
    // 为0~240时表示，只要收到一个同步帧，则将RPDO的数据更新到应用。
    // 为254、255时表示，将接收到的数据直接更新到应用。

    enum class TransferType:uint8_t{
        SyncAcyclic = 0x00,
        SyncMin = 0x01,
        SyncMax = 240,
        RtrSync = 252,
        Rtr = 253,
        EventSpecific = 254,
        EventProfile = 255
    };

    PdoSession(PdoProtocol & pdo, OdEntry & params, OdEntry & mapping)
        : pdo_(pdo), params_(params), mapping_(mapping) {
        cobId = int(params[1].unwarp());
    }


    bool processMessage(const CanMsg& msg);
    bool onSyncEvent();
private:
    PdoProtocol & pdo_;
    OdEntry & params_;
    OdEntry & mapping_;
    CobId cobId;
    int transSyncCount = 0;

    CanMsg buildMessage() const ;
};


}