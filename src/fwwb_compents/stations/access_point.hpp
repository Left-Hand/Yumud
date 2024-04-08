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
        auto msg = CanMsg((uint16_t)((uint8_t)command << 4 | sta_id), buf, len);
        msg.remote(true);
        can.write(msg);
    }

    void sendCommand(const uint8_t & sta_id, const Command & command){
        // logger.println((uint8_t)command, sta_id);
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

    virtual void parseCommand(const uint8_t & id, const Command & cmd, const CanMsg & msg){
        logger.setSpace(" ");
        logger.setEps(10);
        switch(cmd){
        case Command::GET_HP:
            logger.println("H", id, msg[0], msg[1]);
            break;
        case Command::GET_WEIGHT:
            logger.println("W", id, msg[0]);
            break;
        case Command::ATTACK_GET_ID:
            attack_id = msg[0];
            logger.println("I A", msg[0]);
            break;
        case Command::CHASSIS_GET_ID:
            chassis_id = msg[0];
            logger.println("I C", msg[0]);
            break;
        case Command::DEFENSE_GET_ID:
            defense_id = msg[0];
            logger.println("I D", msg[0]);
            break;
        default:
            break;
        }
    }

    virtual void parseCommand(const char & argc, const std::vector<String> & argv){
        logger.setSpace(" ");
        logger.setEps(10);

        switch(argc){

        case 'H':
            if(argv.size() != 1) goto syntax_error;
            {
                const String & targ_node_str = argv[0];
                if(targ_node_str.isNumeric()){
                    uint8_t targ_node_id = int(targ_node_str);
                    sendCommand(targ_node_id, Command::GET_HP);
                }else{
                    uint8_t targ_node_id = 0;
                    switch(targ_node_str[0]){
                    case 'A':
                        targ_node_id = attack_id;
                        break;
                    case 'C':
                        targ_node_id = chassis_id;
                        break;
                    case 'D':
                        targ_node_id = defense_id;
                        break;
                    default:
                        goto syntax_error;
                    }
                    sendCommand(targ_node_id, Command::GET_HP);
                }
            }
            break;

        case 'I'://id
            if(argv.size() > 0){
                switch(argv.size()){
                case 1:
                    switch(argv[0][0]){
                    case 'A':
                        sendCommand(0, Command::ATTACK_GET_ID);
                        break;
                    case 'C':
                        sendCommand(0, Command::CHASSIS_GET_ID);
                        break;
                    case 'D':
                        sendCommand(0, Command::DEFENSE_GET_ID);
                        break;
                    }
                    break;
                case 2:
                    {
                        uint8_t targ_id = int(argv[1]);
                        uint8_t buf[1] = {targ_id};
                        switch(argv[0][0]){
                        case 'A':
                            sendCommand(0, Command::ATTACK_SET_ID, PTR8_AND_SIZE(buf));
                            attack_id = targ_id;
                            break;
                        case 'C':
                            sendCommand(0, Command::CHASSIS_SET_ID, PTR8_AND_SIZE(buf));
                            chassis_id = targ_id;
                            break;
                        case 'D':
                            sendCommand(0, Command::DEFENSE_SET_ID, PTR8_AND_SIZE(buf));
                            defense_id = targ_id;
                            break;
                        }
                    }
                    break;
                default:
                    goto syntax_error;
                }
            }

            else goto syntax_error;
            break;

        case 'A'://Active
            if(argv.size() != 1) goto syntax_error;

            {
                if(int(argv.at(0))){
                    sendCommand(0, Command::ACTIVE);
                }else{
                    sendCommand(0, Command::INACTIVE);
                }
            }

            break;

        case 'Y':
            if(argv.size() != 0) goto syntax_error;

            {
                uint8_t buf[4] = {0};
                uint32_t millis_t = millis();
                memcpy(buf, PTR8_AND_SIZE(millis_t));
                sendCommand(0, Command::SYNC, PTR8_AND_SIZE(buf));
            }
        case 'L'://list module
            if(argv.size() != 0) goto syntax_error;

            {
                logger.println('L', attack_id, chassis_id, defense_id);
            }
            break;

        case 'S'://shot_spec
            // [S] x[0]
            // shot x times
            // [S] p[0]
            // shot type change to p
            if(argv.size() != 2) goto syntax_error;
            {
                uint8_t buf[2];
                buf[0] = int(argv.at(0));
                buf[1] = int(argv.at(1));
                sendCommand(attack_id, Command::ATTACK_SHOT, PTR8_AND_SIZE(buf));
            }
            break;

        case 'T'://Towards
            if(argv.size() != 2) goto syntax_error;
            {
                Vector2 _face = Vector2(real_t(argv.at(0)), real_t(argv.at(1)));
                uint8_t buf[8] = {0};
                // logger.println(_face.x.toString(),_face.x * 1000, real_t(argv.at(0)).toString(), argv.at(0));
                memcpy(&buf, &_face, sizeof(buf));
                sendCommand(attack_id, Command::ATTACK_SET_TOWARD, buf, sizeof(buf));
            }
            break;

        case 'M'://move
            if(argv.size() != 2) goto syntax_error;
            {
                Vector2 _vel = Vector2(real_t(argv.at(0)), real_t(argv.at(1)));
                uint8_t buf[8] = {0};
                memcpy(&buf, &_vel, sizeof(buf));
                sendCommand(chassis_id, Command::CHASSIS_SET_MOVE, buf, sizeof(buf));
            }
            break;

        case 'O'://Omega
            if(argv.size() != 1) goto syntax_error;
            {
                real_t _omega = real_t(argv.at(0));
                uint8_t buf[4] = {0};
                memcpy(&buf, &_omega, sizeof(buf));
                sendCommand(chassis_id, Command::CHASSIS_GET_OMEGA, buf, sizeof(buf));
            }
            break;
        case 'R'://rst
            sendCommand(0, Command::RST);
            break;

        case '?':
            logger.println("OK");
            break;

        syntax_error:
            logger.println("Syntax Error:", argc);
            break;

        default:
            logger.println("Unknown Command:", argc);
            break;
        }

    }
public:
    CanAcessPoint(Can & _can, Printer & _logger) : CanFacility(_can, _logger, 0xf) {;}

    void init(){
        // can.write(CanMsg((uint16_t)((uint8_t)Command::RST << 4), true));
        node_id = 0xf;
        sendCommand(0, Command::RST);
    }



    void run(){
        if(can.available()){
            const CanMsg & msg = can.read();
            uint8_t id = msg.getId() & 0b1111;
            Command cmd = (Command)(msg.getId() >> 4);
            parseCommand(id, cmd, msg);
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