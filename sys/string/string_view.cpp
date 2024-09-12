#include "string_view.hpp"
#include "String.hpp"

StringView::StringView(const String &str):data_(str.c_str()), size_(str.length()){

}


StringView::operator iq_t() const {

    auto [int_part, frac_part, scale] = StringUtils::disassemble_fstr(this->data_, this->size_);

    while(scale > 10000){
        frac_part /= 10;
        scale /= 10;
    }

    return iq_t(int_part) + iq_t(frac_part) / scale;
}
