#ifndef __CAN_MSG_HPP__

#define __CAN_MSG_HPP__

#include "platform.h"
#include <memory.h>
#include <initializer_list>
#include <vector>

class Can1;

struct CanMsg{
protected:
    uint32_t StdId; /* Specifies the standard identifier.
                This parameter can be a value between 0 to 0x7FF. */

    uint32_t ExtId; /* Specifies the extended identifier.
                    This parameter can be a value between 0 to 0x1FFFFFFF. */

    uint8_t IDE; /* Specifies the type of identifier for the message that
                    will be received. This parameter can be a value of
                    @ref CAN_identifier_type */

    uint8_t RTR; /* Specifies the type of frame for the received message.
                    This parameter can be a value of
                    @ref CAN_remote_transmission_request */

    uint8_t DLC; /* Specifies the length of the frame that will be received.
                    This parameter can be a value between 0 to 8 */

    uint8_t Data[8]; /* Contains the data to be received. It ranges from 0 to
                        0xFF. */

    uint8_t FMI; /* Specifies the index of the filter the message stored in
                    the mailbox passes through. This parameter can be a
                    value between 0 to 0xFF */
    uint8_t mbox;

    friend Can1;
    // uint32_t TimeStamp;
public:
    CanMsg():DLC(0){;}
    CanMsg(const uint32_t id, const bool is_rtr = false):StdId(id), ExtId(id), IDE(id > 0x7FF ? CAN_ID_EXT : CAN_ID_STD), RTR(is_rtr ? CAN_RTR_Remote : CAN_RTR_Data), DLC(0){;}

    CanMsg(const uint32_t id, const std::initializer_list<uint8_t> & datas):CanMsg(id){
        for(auto it = datas.begin(); it != datas.end(); it++){
            Data[DLC++] = *it;
        }
    }

    CanMsg(const uint32_t id, const std::vector<uint8_t> &datas) : CanMsg(id) {
        for(auto it = datas.begin(); it != datas.end(); it++){
            Data[DLC++] = *it;
        }
    }

    CanMsg(const uint32_t id, const uint8_t *buf, const uint8_t len) : CanMsg(id) {
        for(uint8_t i = 0; i < len; i++){
            Data[DLC++] = buf[i];
        }
    }

    uint32_t getId() const{
        switch(IDE){
        case CAN_Id_Standard:
            return StdId;
        case CAN_Id_Extended:
            return ExtId;
        default:
            return 0;
        }
    }

    bool isRemote() const {return (RTR == CAN_RTR_Remote);}
    uint8_t getLen() const {return DLC;}
    uint8_t getMailBox() const {return mbox;}

    const uint8_t * getData() const{return Data;}

    // void write(const uint8_t *buf, uint32_t len){
    //     if(RTR == CAN_RTR_Remote) return;
    //     memcpy(Data, buf, len);
    //     DLC = len;
    // }

    // void read(uint8_t *buf, uint32_t len){
    //     if(RTR == CAN_RTR_Remote) return;
    //     memcpy(buf, Data, len);
    //     DLC = len;
    // }

    const uint8_t & operator[](const uint8_t index) const {return *(Data + index);};
    CanTxMsg * toTxMessage() const {return (CanTxMsg *)this;}
    CanRxMsg * toRxMessage() const {return (CanRxMsg *)this;}

};
#endif