#include "StringView.hpp"
#include "String.hpp"


using namespace yumud;

StringView::StringView(const String &str):data_(str.c_str()), size_(str.length()){

}


StringView::operator iq_t() const {
    return StringUtils::atoq(this->data_, this->size_);
}


Strings StringView::split(const char delimiter, const size_t times) const{
    Strings strs;
    
    size_t from = 0;
    for(size_t i = 0; i < this->length(); i++){
        if((i == 0 || data_[i - 1] == delimiter) and data_[i] != delimiter){
            from = i;
        }

        if(data_[i] != delimiter and (i + 1 == length() or data_[i + 1] == delimiter or data_[i + 1] == '\0')){
            strs.push_back(this->substring(from, i+1));
        }

        if((times != 0) and (strs.size() == times)) break;
    }

    return strs;
}


StringView StringView::substring(size_t left, size_t right) const{
	if (left > right) ::std::swap(left, right);
    
	if (left >= size_) return StringView(this->data_, 0);
	if (right > size_) right = size_;

	return StringView(this->data_ + left, right - left);
}


int StringView::indexOf(char c) const
{
	return indexOf(c, 0);
}

int StringView::indexOf( char ch, size_t fromIndex ) const{
	if (fromIndex >= size_) return -1;
	const char* temp = strchr(data_ + fromIndex, ch);
	if (temp == NULL) return -1;
	return temp - data_;
}

// int StringView::lastIndexOf( char theChar ) const
// {
// 	return lastIndexOf(theChar, size_ - 1);
// }

// int StringView::lastIndexOf(char ch, size_t fromIndex) const
// {
// 	if (fromIndex >= size_) return -1;
// 	char tempchar = data_[fromIndex + 1];
// 	buffer[fromIndex + 1] = '\0';
// 	char* temp = strrchr( buffer, ch );
// 	buffer[fromIndex + 1] = tempchar;
// 	if (temp == NULL) return -1;
// 	return temp - buffer;
// }