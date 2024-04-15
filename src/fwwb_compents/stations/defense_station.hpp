#ifndef __FWWB_DEFENSE_STATION_HPP__

#define __FWWB_DEFENSE_STATION_HPP__

#include "target_station.hpp"

namespace FWWB{

#define FATHER_STATION CanStation
class DefenseStation : public FATHER_STATION{
protected:
    static constexpr int panel_size = 8;

    std::array<uint8_t, panel_size> hps;
    PortVirtual<panel_size>leds;
    PortVirtual<panel_size>coil_pins;

    void parseCommand(const Command & cmd, const CanMsg & msg) override{
        // if(!msg.isRemote()) return;
        switch(cmd){
            case Command::DEFENSE_GET_ID:
                {
                    uint8_t buf[1] = {node_id};
                    sendCommand(Command::DEFENSE_GET_ID, PTR8_AND_SIZE(buf));
                }
                break;
            default:
                break;
        }
        FATHER_STATION::parseCommand(cmd, msg);
    }

    void runMachine() override{
        FATHER_STATION::runMachine();
    }
public:

    DefenseStation(FATHER_STATION & _instance): FATHER_STATION(_instance){;}

    void init() override{
        coil_pins.init();
        FATHER_STATION::init();
    }

    void bindPin(const GpioVirtual & gpio, const uint8_t index){
        coil_pins.bindPin(gpio, index);
    }

    void bindPin(const Gpio & gpio, const uint8_t index){
        coil_pins.bindPin(gpio, index);
    }

};
#undef FATHER_STATION
};

#endif