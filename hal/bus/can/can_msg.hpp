#ifndef __CAN_MSG_HPP__

#define __CAN_MSG_HPP__

#include "sys/platform.h"
#include "types/real.hpp"

#include <memory.h>
#include <initializer_list>
#include <vector>
#include <utility>
#include <tuple>


struct CanMsg:public CanRxMsg{
protected:
    uint8_t mbox;
public:
    CanMsg(){
        StdId = 0;
        ExtId = 0;
        IDE = 0;
        RTR = 0;
        DLC = 0;
    }

    CanMsg(const uint32_t id, const bool remote = true){
        StdId = id;
        ExtId = id;
        IDE = (id > 0x7FF ? CAN_ID_EXT : CAN_ID_STD);
        RTR = remote ? CAN_RTR_Remote : CAN_RTR_DATA;
        DLC = 0;
    }

    CanMsg(const uint32_t id, const std::initializer_list<uint8_t> & datas):CanMsg(id, false){
        for(auto it = datas.begin(); it != datas.end(); it++){
            Data[DLC++] = *it;
            if(DLC == 8) break;
        }
    }

    CanMsg(const uint32_t id, const std::vector<uint8_t> &datas) : CanMsg(id, false) {
        for(auto it = datas.begin(); it != datas.end(); it++){
            Data[DLC++] = *it;
            if(DLC == 8) break;
        }
    }

    template <size_t size>
    CanMsg(const uint32_t id, const std::array<uint8_t, size> &datas) : CanMsg(id, false) {
        for(auto it = datas.begin(); it != datas.end(); it++){
            Data[DLC++] = *it;
            if(DLC == 8) break;
        }
    }

    CanMsg(const uint32_t id, const uint8_t *buf, const size_t len) : CanMsg(id, false) {
        for(uint8_t i = 0; i < len; i++){
            Data[DLC++] = buf[i];
            if(DLC == 8) break;
        }
    }

    template<typename T>
    CanMsg(const uint32_t id, const T & value) : CanMsg(id, (const uint8_t *)&value, sizeof(T)) {;}

    operator std::vector<uint8_t>() const{
        std::vector<uint8_t> vec(DLC);
        memcpy(vec.data(), Data, DLC);
        return vec;
    }

    template<size_t size>
    operator std::array<uint8_t, size>() const{
        std::array<uint8_t, size> vec;
        memcpy(vec.data(), Data, size);
        return vec;
    }

    auto to_vector() const {
        return std::vector<uint8_t>(*this);
    }

    template<size_t size>
    auto to_array() const {
        return std::array<uint8_t, size>(*this);
    }

    constexpr bool isStd() const {return IDE == CAN_Id_Standard;}
    constexpr bool isExt() const {return IDE == CAN_Id_Extended;}
    constexpr bool isRemote() const {return (RTR == CAN_RTR_Remote);}
    constexpr uint8_t length() const {return DLC;}
    constexpr uint8_t mailbox() const {return mbox;}

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
    constexpr const uint8_t & operator[](const uint8_t index) const {return *(Data + index);};
    constexpr uint8_t & operator[](const uint8_t index) {return *(Data + index);};

public:
    template<typename T>
    constexpr T to() const {
        T ret;
        msg_memcpy((void *)&ret, (void *)&Data, sizeof(ret), DLC);
        return ret;
    }

    template<typename T>
    constexpr CanMsg & load(const T & para) {
        msg_memcpy((void *)this->Data, (void *)&para, sizeof(para), sizeof(para));
        RTR = CAN_RTR_DATA;
        return *this;
    }

private:
    constexpr static void msg_memcpy(void * dst, const void *src,const size_t len, const size_t dlc){
        for(uint8_t i = 0; i < len; i++){
            ((uint8_t *)dst)[i] = (i < dlc) ? ((const uint8_t *)src)[i] : 0;
        }
    }
};
#endif