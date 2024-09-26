#include "string_view.hpp"
#include "String.hpp"

StringView::StringView(const String &str):data_(str.c_str()), size_(str.length()){

}


StringView::operator iq_t() const {
    return StringUtils::atoq(this->data_, this->size_);
}


Strings StringView::split(const char delimiter, const size_t times) const{
    Strings result;

    int startPos = 0;
    int endPos = indexOf(delimiter, startPos);

    while (endPos != -1) {
        if(not(endPos - startPos <= 1 and this->operator[](startPos) == delimiter)){
            result.push_back(substring(startPos, endPos));
        }
        startPos = endPos + 1;
        endPos = indexOf(delimiter, startPos);
    }

    if (startPos < (int)length()) {
        result.push_back(substring(startPos));
    }

    return result;
}

StringView StringView::substring(size_t left, size_t right) const{
	if (left > right) {
		size_t temp = right;
		right = left;
		left = temp;
	}
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