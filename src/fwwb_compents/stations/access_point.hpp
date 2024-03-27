#ifndef __FWWB_CAN_ACCESS_POINT_HPP__

#define __FWWB_CAN_ACCESS_POINT_HPP__

#include "../can_station.hpp"

namespace FWWB{
class CanAcessPoint:public CanFacility{
protected:
    enum class StateMachine:uint8_t{
        POWER_ON,
        ACTIVE
    };

    volatile StateMachine sm = StateMachine::POWER_ON;

    void sendCommand(const uint8_t & sta_id, const Command & command, const uint8_t *buf, const uint8_t len){
        can.write(CanMsg((uint16_t)((uint8_t)command << 4 | sta_id), buf, len));
    }

    void sendCommand(const uint8_t & sta_id, const Command & command){
        can.write(CanMsg((uint16_t)((uint8_t)command << 4 | sta_id), true));
    }

    bool sta_valid[14] = {false};

    void scanStations(){
        struct MsgFormat{
            union{
                uint8_t buf[4];
                uint32_t val;
            };
            uint8_t node_id;
        };

        for(uint8_t i = 0; i < 14; i++){
            MsgFormat content{.val = sta_crcs[i], .node_id = (uint8_t)(i + 1)};
            sendCommand(i + 1, Command::SCAN, PTR8_AND_SIZE(content));
        }
    }

    std::vector<String> splitString(const String& input, char delimiter) {
        std::vector<String> result;

        int startPos = 0;
        int endPos = input.indexOf(delimiter, startPos);

        while (endPos != -1) {
            String token = input.substring(startPos, endPos);
            result.push_back(token.c_str());

            startPos = endPos + 1;
            endPos = input.indexOf(delimiter, startPos);
        }

        if (startPos < input.length()) {
            String lastToken = input.substring(startPos);
            result.push_back(lastToken.c_str());
        }

        return result;
    }

    String temp_str = "";
    uint8_t chassis_id = 0;
    uint8_t attack_id = 0;
    uint8_t defense_id = 0;

    virtual void parseCommand(const Command & cmd, const CanMsg & msg){
        switch(cmd){
        case Command::SCAN:

        default:
            break;
        }
    }

    virtual void parseCommand(const char & argc, const std::vector<String> & argv){
        switch(argc){
        case 'L'://list module
            {
                logger.setSpace(" ");
                logger.print("A", attack_id);
                logger.print("C", chassis_id);
                logger.println("D", defense_id);
            }
        case 'S':
            // [S] x[0]
            // shot x times
            {
                uint8_t buf[1];
                buf[0] = int(argv.at(0));
                sendCommand(attack_id, Command::ATTACK_SHOT, PTR8_AND_SIZE(buf));
            }
            break;
        case 'H'://shot_spec

            // [H] p[0]
            // shot type change to p
            {
                uint8_t buf[1];
                buf[0] = int(argv.at(0));
                sendCommand(attack_id, Command::ATTACK_SET_SHOT_SPEC, PTR8_AND_SIZE(buf));
            }
        case 'F'://face
            {
                Vector2 _face = Vector2(real_t(argv.at(0)), real_t(argv.at(1)));
                uint8_t buf[8] = {0};
                memcpy(&buf, &_face, sizeof(buf));
                sendCommand(chassis_id, Command::ATTACK_SET_FACE, buf, sizeof(buf));
            }
            break;
        case 'V'://velocity
            {
                Vector2 _vel = Vector2(real_t(argv.at(0)), real_t(argv.at(1)));
                uint8_t buf[8] = {0};
                memcpy(&buf, &_vel, sizeof(buf));
                sendCommand(chassis_id, Command::CHASSIS_SET_VEL, buf, sizeof(buf));
            }
            break;
        case 'O'://Omega
            {
                real_t _omega = real_t(argv.at(0));
                uint8_t buf[4] = {0};
                memcpy(&buf, &_omega, sizeof(buf));
                sendCommand(chassis_id, Command::CHASSIS_GET_OMEGA, buf, sizeof(buf));
            }
            break;
        case 'R'://rst
            sendCommand(0, Command::RST);
        default:
            logger.println("Unknown command:", argc);
        }
    }
public:
    CanAcessPoint(Can & _can, Printer & _logger) : CanFacility(_can, _logger, 0xf) {;}

    void init(){
        // can.write(CanMsg((uint16_t)((uint8_t)Command::RST << 4), true));
        sendCommand(0, Command::RST);
    }



    void run(){
        if(can.available()){
            const CanMsg & msg = can.read();
            uint8_t id = msg.getId() & 0b1111;
            if(id == 0 || id == node_id){
                Command cmd = (Command)(msg.getId() >> 4);
                parseCommand(cmd, msg);
            }
        }

        if(logger.available()){
            char chr = logger.read();
            if(chr == '\n'){
                temp_str.trim();
                auto tokens = splitString(temp_str, ' ');
                auto argc = tokens[0][0];
                tokens.erase(tokens.begin());
                parseCommand(argc, tokens);
                temp_str = "";
            }else{
                temp_str.concat(chr);
            }
        }
    }
};
};

#endif