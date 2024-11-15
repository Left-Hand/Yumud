#pragma once
namespace ymd{
class StringView;
}

namespace gxm{

class Lcd{
public:
    Lcd() = default;
    
    void showString(const ymd::StringView & str);
};

}