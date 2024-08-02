#include "cli.hpp"


namespace StepperUtils{
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