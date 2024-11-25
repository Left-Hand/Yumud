#pragma once

#include "sys/core/platform.h"
#include "sys/stream/ostream.hpp"

#include <memory.h>
#include <initializer_list>
#include <vector>
#include <utility>
#include <tuple>

namespace ymd{

//TODO do not inhert from CanRxMsg
struct CanMsg{
protected:
    uint32_t StdId;  /* Specifies the standard identifier.
                        This parameter can be a value between 0 to 0x7FF. */

    uint32_t ExtId;  /* Specifies the extended identifier.
                        This parameter can be a value between 0 to 0x1FFFFFFF. */

    uint8_t IDE;     /* Specifies the type of identifier for the message that 
                        will be received. This parameter can be a value of 
                        @ref CAN_identifier_type */

    uint8_t RTR;     /* Specifies the type of frame for the received message.
                        This parameter can be a value of 
                        @ref CAN_remote_transmission_request */

    uint8_t DLC;     /* Specifies the length of the frame that will be received.
                        This parameter can be a value between 0 to 8 */

    uint8_t Data[8]; /* Contains the data to be received. It ranges from 0 to 
                        0xFF. */

    uint8_t FMI;     /* Specifies the index of the filter the message stored in 
                        the mailbox passes through. This parameter can be a 
                        value between 0 to 0xFF */
    uint8_t mbox;

public:
    constexpr CanMsg(){
        // StdId = 0;
        // ExtId = 0;
        // IDE = 0;
        // RTR = 0;
        // DLC = 0;
    }

    constexpr CanMsg(const CanMsg & other) = default;
    constexpr CanMsg(CanMsg && other) = default;
    constexpr CanMsg & operator = (const CanMsg & other) = default;
    constexpr CanMsg & operator = (CanMsg && other) = default;

    CanMsg copy(){
        return *this;
    }

    constexpr CanMsg(const uint32_t id, const bool remote = true){
        StdId = id;
        ExtId = id;
        IDE = (id > 0x7FF ? CAN_ID_EXT : CAN_ID_STD);
        RTR = remote ? CAN_RTR_Remote : CAN_RTR_DATA;
        DLC = 0;
    }

    template<typename T>
    requires (sizeof(T) <= 8) and (!std::is_pointer_v<T>)
    constexpr CanMsg & operator << (const T & val){
        for(size_t i = 0; i < sizeof(T) and DLC < 8; i++){
            Data[DLC++] = ((const uint8_t *)&val)[i];
        }
        RTR = CAN_RTR_Data;
        return *this;
    }

    // 输入流运算符重载
    template<typename T>
    requires (sizeof(T) <= 8)
    constexpr CanMsg & operator>>(T && val) {
        if (DLC < sizeof(T)-1) {
            return *this;
        }
        for (size_t i = 0; i < sizeof(T); i++) {
            ((uint8_t *)&val)[i] = Data[i];
        }
        // 更新 DLC，假设读取后清空数据
        DLC -= sizeof(T);
        for (size_t i = 0; i < DLC; i++) {
            Data[i] = Data[i + sizeof(T)];  // 移动剩余数据
        }
        return *this;
    }


    template <typename... Args>
    requires (sizeof(std::tuple<Args...>) <= 8) and (!std::disjunction_v<std::is_pointer<Args>...>)
    constexpr CanMsg(const uint32_t id, const std::tuple<Args...>& tup):CanMsg(id) {
        std::apply(
            [&](auto&&... args) {
                ((*this << args), ...);
            }, tup
        );

    }

    constexpr CanMsg(const uint32_t id, const uint8_t *buf, const size_t len) : CanMsg(id) {
        setSize(MIN(len, 8));

        for(uint8_t i = 0; i < size(); i++){
            this->operator[](i) = buf[i];
        }
    }

    constexpr uint8_t * begin(){return Data;}
    constexpr uint8_t * end(){return Data + size();}
    constexpr const uint8_t * begin() const {return Data;}
    constexpr const uint8_t * end() const {return Data + size();}
    constexpr size_t size() const {return MIN(DLC, 8);}

    operator std::vector<uint8_t>() const{return std::vector<uint8_t>{begin(), end()};}

    template<size_t N>
    operator std::array<uint8_t, N>() const{
        std::array<uint8_t, N> ret;
        for(size_t i = 0; i < N; i++){
            ret[i] = (*this)[i];
        }
        return ret;
    }

    auto to_vector() const {
        return std::vector<uint8_t>(*this);
    }

    template<size_t N>
    auto to_array() const {
        return std::array<uint8_t, N>(*this);
    }

    constexpr bool isStd() const {return IDE == CAN_Id_Standard;}
    constexpr bool isExt() const {return IDE == CAN_Id_Extended;}
    constexpr bool isRemote() const {return (RTR == CAN_RTR_Remote);}
    constexpr uint8_t mailbox() const {return mbox;}
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
    constexpr const uint8_t operator[](const int index) const {return *(Data + index);};
    constexpr uint8_t & operator[](const int index) {return *(Data + index);};


    template<typename T>
    requires (sizeof(T) <= 8)
    constexpr operator T () const {
        T ret;
        memcpy((void *)&ret, &Data, MIN(sizeof(ret), size()));
        return ret;
    }

    
    void setExt(const bool en){
        IDE = (en ? CAN_ID_EXT : CAN_ID_STD);
    }
    
    constexpr void setSize(const size_t size) {DLC = size;}


    #ifdef HDW_SXX32

    CanTxMsg * ptx() {
        return reinterpret_cast<CanTxMsg *>(this);
    }

    CanRxMsg * prx() {
        return reinterpret_cast<CanRxMsg *>(this);
    }

    const CanTxMsg * cptx() const {
        return reinterpret_cast<const CanTxMsg *>(this);
    }

    const CanRxMsg * cprx() const {
        return reinterpret_cast<const CanRxMsg *>(this);
    }


    #endif
};

__inline OutputStream & operator<<(OutputStream & os, const CanMsg & msg){
    os << "{" << std::showbase << 
        std::hex << msg.id() << '<'
        << ((msg.isStd()) ? "Std" : "Ext")
        << ((msg.isRemote()) ? "Rmt" : "Dat") << std::noshowbase
        << '[' << std::dec << msg.size() << ']';
    os << "> ";
    
    os << std::hex;

    for(size_t i = 0; i < msg.size(); i++){
        os << msg[i];
        if(i == msg.size() - 1) break;
        os << ',';
    }
    
    os << std::dec;
        
    return os << '}';
}
}
