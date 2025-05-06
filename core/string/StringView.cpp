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

std::optional<size_t> StringView::find(char c) const{
	return find_from(c, 0);
}

std::optional<size_t> StringView::find_from(char ch, size_t from) const{
	if (from >= size_) return std::nullopt;
    for(size_t i = from; i < size_; i++){
        if(data_[i] == ch) return i;
    }
    return std::nullopt;
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