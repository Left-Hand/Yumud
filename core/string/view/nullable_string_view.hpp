#pragma once

namespace ymd{
#if 0
struct NullableStringView{
    static constexpr NullableStringView from_null(){
        return NullableStringView(nullptr);
    }

    constexpr NullableStringView() = delete;
    constexpr NullableStringView(const char * str) = delete;

    constexpr explicit NullableStringView(std::nullptr_t):
        data_(nullptr),
        size_(0){}

    constexpr NullableStringView(_None_t):
        data_(nullptr),
        size_(0){}

    constexpr NullableStringView(const StringView & str):
        data_(str.data()),
        size_(str.length()){}

    constexpr NullableStringView(Some<StringView> str):
        NullableStringView(str.get()){;}

    constexpr NullableStringView(Option<StringView> may_str){
        if(may_str.is_some()){
            const auto str = may_str.unwrap();
            data_ = str.data();
            size_ = str.size();
        }else{
            data_ = nullptr;
            size_ = 0;
        }
    }


    constexpr explicit NullableStringView(const NullableStringView & other):
        data_(other.data_),
        size_(other.size_){}
    constexpr explicit NullableStringView(NullableStringView && other):
        data_(other.data_),
        size_(other.size_){}

    constexpr bool is_null() const{return size_==0;}
    constexpr bool is_none() const{return size_==0;}
    constexpr bool is_some() const{return size_!=0;}

    constexpr StringView unwrap_or(const StringView default_str) const{
        if(unlikely(size_ == 0)) return default_str;
        return unwrap();
    }

    constexpr StringView unwrap() const{
        if(unlikely(size_ == 0)) __builtin_trap();
        return StringView(data_, size_);
    }

    constexpr StringView unwrap_unchecked() const{
        return StringView(data_, size_);
    }

    constexpr size_t length() const{
        return size_;
    }

private:
    const char * data_;
    size_t size_;
};
#endif
}