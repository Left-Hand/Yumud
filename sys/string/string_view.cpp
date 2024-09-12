#include "string_view.hpp"
#include "String.hpp"

StringView::StringView(const String &str):data_(str.c_str()), size_(str.length()){

}
