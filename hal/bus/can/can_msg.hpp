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
        DLC = 0;
    }

    CanMsg(const uint32_t id, const bool is_rtr = false){
        StdId = id;
        ExtId = id;
        IDE = (id > 0x7FF ? CAN_ID_EXT : CAN_ID_STD);
        RTR = (is_rtr ? CAN_RTR_Remote : CAN_RTR_Data);
        DLC = 0;
    }

    CanMsg(const uint32_t id, const std::initializer_list<uint8_t> & datas):CanMsg(id){
        for(auto it = datas.begin(); it != datas.end(); it++){
            Data[DLC++] = *it;
            if(DLC == 8) break;
        }
    }

    CanMsg(const uint32_t id, const std::vector<uint8_t> &datas) : CanMsg(id) {
        for(auto it = datas.begin(); it != datas.end(); it++){
            Data[DLC++] = *it;
            if(DLC == 8) break;
        }
    }

    CanMsg(const uint32_t id, const uint8_t *buf, const size_t len) : CanMsg(id) {
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

private:
    using tuple_1 = std::tuple<real_t, real_t>;
    using tuple_2 = std::tuple<float, float>;
    using tuple_3 = std::tuple<real_t, real_t>;
    using tuple_4 = std::tuple<real_t, real_t>;

public:
    #define CONV_FUNC(type)\
    explicit operator type () const{\
        type ret;\
        msg_memcpy((void *)&ret, (void *)&Data, sizeof(type), DLC);\
        return ret;\
    } \
    \
    explicit CanMsg(type && para){\
        msg_memcpy((void *)&Data, (void *)&para, sizeof(type), DLC);\
        DLC = sizeof(type) / sizeof(uint8_t);\
        if(DLC > 8) DLC = 8;\
    }\

    #define CONV_TUPLE(...)\
    explicit operator std::tuple<__VA_ARGS__> () const{\
        std::tuple<__VA_ARGS__> ret;\
        msg_memcpy((void *)&ret, (void *)&Data, sizeof(ret), DLC);\
        return ret;\
    } \
    \
    explicit CanMsg(std::tuple<__VA_ARGS__> && para){\
        msg_memcpy((void *)&Data, (void *)&para, sizeof(para), DLC);\
        DLC = sizeof(para) / sizeof(uint8_t);\
        if(DLC > 8) DLC = 8;\
    }\

    CONV_FUNC(real_t)
    CONV_FUNC(float)
    CONV_FUNC(int)


    CONV_TUPLE(real_t, real_t)
    CONV_TUPLE(float, float)
    CONV_TUPLE(int, int)

    template<typename T>
    static CanMsg from(T && para){
        CanMsg ret;
        msg_memcpy((void *)&ret.Data, (void *)&para, sizeof(para), 8);
        ret.DLC = MIN(sizeof(para), 8);
        return ret;
    }

    template<typename T>
    T to() const {
        T ret;
        msg_memcpy((void *)&ret, (void *)&Data, sizeof(ret), DLC);
        return ret;
    }

    template<typename T>
    CanMsg & load(T && para) {
        msg_memcpy((void *)this, (void *)&para, sizeof(para), DLC);
        return *this;
    }

    #undef CONV_FUNC
    #undef CONV_TUPLE
protected:
    static void msg_memcpy(void * dst, const void *src,const size_t len, const size_t dlc){
        for(uint8_t i = 0; i < len; i++){
            ((uint8_t *)dst)[i] = (i < dlc) ? ((const uint8_t *)src)[i] : 0;
        }
    }
};
#endif