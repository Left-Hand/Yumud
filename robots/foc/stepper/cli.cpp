#include "cli.hpp"


namespace StepperUtils{

std::vector<String> Cli::split_string(const String& input, char delimiter) {
    std::vector<String> result;

    int startPos = 0;
    int endPos = input.indexOf(delimiter, startPos);

    while (endPos != -1) {
        if(not(endPos - startPos <= 1 and input[startPos] == delimiter)){
            String token = input.substring(startPos, endPos);
            result.push_back(token.c_str());
        }
        startPos = endPos + 1;
        endPos = input.indexOf(delimiter, startPos);
    }

    if (startPos < (int)input.length()) {
        String lastToken = input.substring(startPos);
        result.push_back(lastToken.c_str());
    }

    return result;
}

void Cli::parseTokens(const String & _command,const std::vector<String> & args){
    auto command = _command;
    switch(hash_impl(command.c_str(), command.length())){
        case "reset"_ha:
        case "rst"_ha:
        case "r"_ha:
            CLI_PRINTS("rsting");
            NVIC_SystemReset();
            break;
        case "alive"_ha:
        case "a"_ha:
            CLI_PRINTS("chip is alive");
            break;
        default:
            CLI_PRINTS("no command available:", command);
            break;
    }
}

void Cli::parseLine(const String & _line){
    if(_line.length() < 1) return;

    auto tokens = split_string(_line, ' ');
    if(tokens.size()){
        auto command = tokens[0];
        tokens.erase(tokens.begin());
        parseTokens(command, tokens);
    }
}

}