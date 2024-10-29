#pragma once

class String;

namespace gxm{

class Lcd{
public:
    Lcd() = default;
    
    void showString(const String & str);
};

}