#ifndef __CAN_MSG_HPP__

#define __CAN_MSG_HPP__

#include "sys/platform.h"
#include <memory.h>
#include <initializer_list>
#include <vector>

class Can;

struct CanMsg:public CanRxMsg{
protected:
    uint8_t mbox;

    friend Can;
public:
    CanMsg(){
        DLC = 0;
    }

    CanMsg(const uint32_t & id, const bool & is_rtr = false){
        StdId = id;
        ExtId = id;
        IDE = (id > 0x7FF ? CAN_ID_EXT : CAN_ID_STD);
        RTR = (is_rtr ? CAN_RTR_Remote : CAN_RTR_Data);
        DLC = 0;
    }

    CanMsg(const uint32_t & id, const std::initializer_list<uint8_t> & datas):CanMsg(id){
        for(auto it = datas.begin(); it != datas.end(); it++){
            Data[DLC++] = *it;
            if(DLC == 8) break;
        }
    }

    CanMsg(const uint32_t & id, const std::vector<uint8_t> &datas) : CanMsg(id) {
        for(auto it = datas.begin(); it != datas.end(); it++){
            Data[DLC++] = *it;
            if(DLC == 8) break;
        }
    }

    CanMsg(const uint32_t & id, const uint8_t *buf, const uint8_t len) : CanMsg(id) {
        for(uint8_t i = 0; i < len; i++){
            Data[DLC++] = buf[i];
            if(DLC == 8) break;
        }
    }

    bool isStd() const {return IDE == CAN_Id_Standard;}
    bool isExt() const {return IDE == CAN_Id_Extended;}
    bool isRemote() const {return (RTR == CAN_RTR_Remote);}
    uint8_t length() const {return DLC;}
    uint8_t mailbox() const {return mbox;}

    void write(const uint8_t *buf, size_t len){
        if(RTR == CAN_RTR_Remote) return;
        len = MIN(len, 8);
        memcpy(Data, buf, len);
        DLC = len;
    }

    void read(uint8_t * buf, size_t len){
        if(RTR == CAN_RTR_Remote) return;
        len = MIN(len, 8);
        memcpy(buf, Data, len);
        DLC = len;
    }

    uint32_t id() const {
        if(isStd()) return StdId;
        else if(isExt()) return ExtId;
        else return 0; 
    }
    const uint8_t & operator[](const uint8_t index) const {return *(Data + index);};
};
#endif