#include "StringView.hpp"
#include "String.hpp"


using namespace ymd;

std::vector<StringView> StringView::split(const char delimiter, const size_t max_pieces) const{
    std::vector<StringView> strs;
    
    size_t from = 0;
    for(size_t i = 0; i < this->length(); i++){
        if((i == 0 || data_[i - 1] == delimiter) and data_[i] != delimiter){
            from = i;
        }

        if(data_[i] != delimiter and (i + 1 == length() or data_[i + 1] == delimiter or data_[i + 1] == '\0')){
            strs.push_back(this->substr(from, i+1));
        }

        if((max_pieces != 0) and (strs.size() == max_pieces)) break;
    }

    return strs;
}

// int StringView::indexOf(char c) const
// {
// 	return indexOf(c, 0);
// }

// int StringView::indexOf( char ch, size_t fromIndex ) const{
// 	if (fromIndex >= size_) return -1;
// 	const char* temp = strchr(data_ + fromIndex, ch);
// 	if (temp == NULL) return -1;
// 	return temp - data_;
// }

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