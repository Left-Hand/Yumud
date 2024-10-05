#ifndef __CAN_MSG_HPP__

#define __CAN_MSG_HPP__

#include "sys/core/system.hpp"

#include <memory.h>
#include <initializer_list>
#include <vector>
#include <utility>
#include <tuple>


struct CanMsg:public CanRxMsg{
protected:
    uint8_t mbox;
public:
    constexpr CanMsg(){
        StdId = 0;
        ExtId = 0;
        IDE = 0;
        RTR = 0;
        DLC = 0;
    }

    constexpr CanMsg(const CanMsg & other) = default;
    constexpr CanMsg(CanMsg && other) noexcept = default;
    constexpr CanMsg & operator = (const CanMsg & other) = default;

    constexpr CanMsg(const uint32_t id, const bool remote = true){
        StdId = id;
        ExtId = id;
        IDE = (id > 0x7FF ? CAN_ID_EXT : CAN_ID_STD);
        RTR = remote ? CAN_RTR_Remote : CAN_RTR_DATA;
        DLC = 0;
    }

    template<typename T>
    requires (sizeof(T) <= 8) and (!std::is_pointer_v<T>)
    constexpr CanMsg & operator << (T && val){
        for(size_t i = 0; i < sizeof(T) and DLC < 8; i++){
            Data[DLC++] = ((uint8_t *)&val)[i];
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

    operator std::vector<uint8_t>() const{return {begin(), end()};}

    template<size_t N>
    operator std::array<uint8_t, N>() const{return {begin(), end()};}

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
    constexpr const uint8_t & operator[](const int index) const {return *(Data + index);};
    constexpr uint8_t & operator[](const int index) {return *(Data + index);};

    constexpr const uint8_t & at(const int index) const {
        if(IN_RANGE(index, 0, (DLC-1))){
            return Data[index];
        }else{
            return null_data;
        }
    };

    template<typename T>
    requires (sizeof(T) <= 8)
    constexpr operator T () const {
        T ret;
        memcpy((void *)&ret, (void *)&Data, MIN(sizeof(ret), size()));
        return ret;
    }

    
    void setExt(const bool en){
        IDE = (en ? CAN_ID_EXT : CAN_ID_STD);
    }
    
    constexpr void setSize(const size_t size) {DLC = size;}
private:
    scexpr uint8_t null_data = 0;
};

struct OutputStream;

OutputStream & operator<<(OutputStream & os, const CanMsg & msg);
#endif