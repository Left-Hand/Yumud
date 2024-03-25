
#ifndef __FWWB_TARGET_STATION_HPP__

#define __FWWB_TARGET_STATION_HPP__

#include "../can_station.hpp"
#include "../panel.hpp"

namespace FWWB{
class TargetStation:public CanStation{
protected:
    PanelUnit & unit0;
    PanelUnit & unit1;

    bool Shotted(){
        return unit0.isShotted() || unit1.isShotted();
    }

    bool Outline(){
        return unit0.isOutline() || unit1.isOutline();
    }

    bool Died(){
        return unit0.isDied() && unit1.isDied();
    }

    void OutLineNotify(){
        sendCommand(Command::OUTBOUND);
    }

    void HpNotify(){
        uint8_t buf[2] = {unit0.hp, unit1.hp};
        sendCommand(Command::HP, VAR_AND_SIZE(buf));
    }

    void HpNotified(const CanMsg & msg){
        unit0.hp = msg.getData()[0];
        unit1.hp = msg.getData()[1];
    }

    void runMachine() override{
        unit0.run();
        unit1.run();

        switch(sm){
        case StateMachine::ACTIVE:

            if(Outline()){
                OutLineNotify();
                sm = StateMachine::INACTIVE;
            }else if(Shotted()){
                HpNotify();
                if(Died()){
                    sm = StateMachine::INACTIVE;
                }
            }

            break;
        default:
            CanStation::runMachine();
            break;
        }
    }

    void parseCommand(const Command & cmd, const CanMsg & msg) override{

        switch(cmd){
        case Command::HP:
            if(msg.isRemote())
                HpNotify();
            else
                HpNotified(msg);
            break;

        default:
            CanStation::parseCommand(cmd, msg);
            break;
        }
    }
public:

    virtual void init(){
        unit0.init();
        unit1.init();
    }

    TargetStation(CanStation & _can_station, PanelUnit & _unit0, PanelUnit & _unit1) : CanStation(_can_station), unit0(_unit0), unit1(_unit1){;}
};
};

#endif