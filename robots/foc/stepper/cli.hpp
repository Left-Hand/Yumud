#ifndef __STEPPER_CLI_HPP__

#define __STEPPER_CLI_HPP__

#include "sys/core/system.hpp"
#include "motor_utils.hpp"
#include "statled.hpp"
#include "hal/bus/can/can.hpp"

namespace MotorUtils{
    class Cli{
    private:
        std::vector<String> split_string(const String& input, char delimiter);

    protected:
        IOStream & logger;
        Can & can;
        NodeId node_id;
        using Command = MotorUtils::Command;
    public:
        Cli(IOStream & _logger, Can & _can, const NodeId _node_id):logger(_logger), can(_can), node_id(_node_id){;}

        #define VNAME(x) #x

        virtual void parseTokens(const String & _command,const std::vector<String> & args);
        void parseLine(const String & _line);

        virtual void readCan() = 0;
    };

    class CliSTA : public Cli{
    public:
        void readCan() override{
            if(can.available()){
            const CanMsg & msg = can.read();
            uint8_t id = msg.id() >> 7;
            Command cmd = (Command)(msg.id() & 0x7F);
            if(id == 0 || id == uint8_t(node_id)){
                parseCommand(cmd, msg);
            }
    }
        }
        CliSTA(IOStream & _logger, Can & _can, const NodeId _node_id):Cli(_logger, _can, _node_id){;}
        virtual void parseCommand(const Command command, const CanMsg & msg){
            switch(command){
                case Command::RST:
                    Sys::Misc::reset();
                    break;
                default:
                    break;
            }
        }
    };

    class CliAP : public Cli{
    public:
        void readCan() override{
            if(can.available()){
                const CanMsg & msg = can.read();
                uint8_t id = msg.id() >> 7;
                Command cmd = (Command)(msg.id() & 0x7f);
                parseCommand(id, cmd, msg);
            }
        }
        CliAP(IOStream & _logger, Can & _can):Cli(_logger, _can, 0x0f){;}
        virtual void parseCommand(const NodeId id, const Command cmd, const CanMsg & msg) = 0;
    };

}

#endif