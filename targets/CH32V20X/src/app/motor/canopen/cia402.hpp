#pragma once

#include "utils.hpp"

namespace ymd::canopen{

enum class CiA402State : uint8_t {
    NotReadyToSwitchOn = 0,
    SwitchOnDisabled = 1,
    ReadyToSwitchOn = 2,
    SwitchedOn = 3,
    OperationEnabled = 4,
    QuickStopActive = 5,
    FaultReactionActive = 6,
    Fault = 7
};


struct ControlWord {
    uint8_t switchOn : 1;
    uint8_t enableVoltage : 1;
    uint8_t quickStop : 1;
    uint8_t enableOperation : 1;
    uint8_t faultReset : 1;
    uint8_t halt : 1;
    uint8_t :2;
};

struct StatusWord {
    uint8_t readyToSwitchOn : 1;
    uint8_t switchedOn : 1;
    uint8_t operationEnabled : 1;
    uint8_t fault : 1;
    uint8_t voltageEnabled : 1;
    uint8_t quickStop : 1;
    uint8_t warning : 1;
    uint8_t targetReached : 1;
};


enum class MotionMode : uint8_t {
    NoMode = 0,
    ProfilePosition = 1,
    ProfileVelocity = 3,
    Torque = 4
};


class CanopenNode {
private:
    void handleFrame(const CanMsg& frame) {
        // 处理接收到的CAN帧
        // 这里可以根据CANopen协议解析帧并更新对象字典或状态机
    }

    CanDriver& driver_;
    uint8_t nodeId_;
    uint8_t state = 0; // 初始状态
    ObjectDictionary objectDict_;
public:
    CanopenNode(CanDriver& driver, uint8_t nodeId) : driver_(driver), nodeId_(nodeId) {}

    void process() {
        // CanMsg frame;
        // while (driver.receive(frame)) {
        //     handleFrame(frame);
        // }
    }

    void sendHeartbeat() {
        // CanMsg frame;
        // frame.id = 0x700 + nodeId;
        // frame.data[0] = state;
        // frame.len = 1;
        // driver.send(frame);
    }

    void setState(uint8_t newState) {
        state = newState;
    }

    ObjectDictionary& dict() {
        return objectDict_;
    }

    const ObjectDictionary& dict() const {
        return objectDict_;
    }


};

class CiA402Device {
public:
    CiA402Device(CanDriver& driver, uint8_t nodeId)
    : node_(driver, nodeId), currentState(CiA402State::SwitchOnDisabled) 
    {
        // 初始化对象字典
        node_.dict().set({0x6040, 0x00}, static_cast<uint16_t>(0)); // 控制字
        node_.dict().set({0x6041, 0x00}, static_cast<uint16_t>(0)); // 状态字
        node_.dict().set({0x6060, 0x00}, static_cast<uint8_t>(0));  // 运动模式
    }

    void process() {
        node_.process();
        updateStateMachine();
    }

    void setControlWord(uint16_t controlWord) {
        node_.dict().set({0x6040, 0x00}, controlWord);
    }

    uint16_t getStatusWord() const {
        return std::get<uint16_t>(node_.dict().get({0x6041, 0x00}).value());
    }

    void setMotionMode(MotionMode mode) {
        node_.dict().set({0x6060, 0x00}, static_cast<uint8_t>(mode));
    }

    void setTargetPosition(int32_t position) {
        node_.dict().set({0x607A, 0x00}, position);
    }

    void setTargetVelocity(int32_t velocity) {
        node_.dict().set({0x60FF, 0x00}, velocity);
    }

    void setTargetTorque(int16_t torque) {
        node_.dict().set({0x6071, 0x00}, torque);
    }

private:
    void updateStateMachine() {
        uint16_t statusWord = getStatusWord();
        switch (currentState) {
            case CiA402State::SwitchOnDisabled:
                if (statusWord & 0x0040) { // 检查“Switch on disabled”状态
                    currentState = CiA402State::ReadyToSwitchOn;
                }
                break;
            case CiA402State::ReadyToSwitchOn:
                if (statusWord & 0x0021) { // 检查“Ready to switch on”状态
                    currentState = CiA402State::SwitchedOn;
                }
                break;
            case CiA402State::SwitchedOn:
                if (statusWord & 0x0023) { // 检查“Switched on”状态
                    currentState = CiA402State::OperationEnabled;
                }
                break;
            case CiA402State::OperationEnabled:
                if (statusWord & 0x0027) { // 检查“Operation enabled”状态
                    // 正常运行
                }
                break;
            case CiA402State::Fault:
                if (statusWord & 0x000F) { // 检查“Fault”状态
                    // 故障处理
                }
                break;
            default:
                break;
        }
    }

    CanopenNode node_;
    CiA402State currentState;
};

}