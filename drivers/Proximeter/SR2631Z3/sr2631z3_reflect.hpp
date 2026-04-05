#pragma once

#include "sr2631z3_prelude.hpp"

namespace ymd::drivers::sr2631z3{


template<typename T>
struct FieldsTraiter{
    static constexpr size_t NUM_FIELDS = reflect::size<T>();


    template<size_t I>
    static constexpr auto & get_element(T & self){
        return reflect::get<I>(self);
    }


    template<size_t I>
    static constexpr const auto & get_element(const T & self){
        return reflect::get<I>(self);
    }
};


template<typename T>
struct [[nodiscard]] FieldParser{
    using Error = DeMsgErrorKind;

    static constexpr Result<void, Error> parse_field(T & field, const StringView str){
        if(str.length() == 0) return Err(Error::NonnullObject);

        field = ({
            const auto res = T::try_from_str(str);
            if(res.is_err()) return Err(res.unwrap_err());
            (res.unwrap());
        });
        return Ok();
    }
};

template<typename T>
struct [[nodiscard]] FieldParser<Option<T>>{
    using Error = DeMsgErrorKind;

    static constexpr Result<void, Error> parse_field(Option<T> & field, const StringView str){
        if(str.length() == 0){
            field = None;
            return Ok();
        }

        field = ({
            const auto res = T::try_from_str(str);
            if(res.is_err()) return Err(res.unwrap_err());
            Some(res.unwrap());
        });
        return Ok();
    }
};



template<typename T>
struct DeserializeHelper {
    template <size_t I, size_t... Js>
    static constexpr Result<void, DeMsgErrorKind>
    parse_remeaning(T & self, StringSplitIter splitter, std::index_sequence<I, Js...>) {
        if(splitter.has_next() == false) return Err(DeMsgErrorKind::MissingField); 
        const auto str = splitter.next();
        auto & field = FieldsTraiter<T>::template get_element<I>(self);
        using FieldType = std::decay_t<decltype(field)>;
        if(const auto res = FieldParser<FieldType>::parse_field(field, str);
            res.is_err()) return Err(res.unwrap_err());
        return parse_remeaning(self, std::move(splitter), std::index_sequence<Js...>{});
    }

    template <typename... Args>
    static constexpr Result<void, DeMsgErrorKind>
    parse_remeaning(
        [[maybe_unused]] T & self, 
        StringSplitIter splitter, 
        [[maybe_unused]] std::index_sequence<>
    ) {
        if(splitter.has_next()) return Err(DeMsgErrorKind::TooMuchFields);
        return Ok();
    }

private:
    //空时返回nullptr
    [[nodiscard]] const char * search_comma(const StringView str){
        const auto it = std::find(str.begin(), str.end(), ',');
        if(it == str.end()) return nullptr;
        return static_cast<const char *>(it);
    }
};


template<typename T>
static constexpr Result<T, DeMsgErrorKind> try_deser_msg(const StringView context_str){
    static constexpr size_t NUM_FIELDS = FieldsTraiter<T>::NUM_FIELDS;
    T self = T::from_uninitialized();

    if(const auto res = DeserializeHelper<T>::parse_remeaning(
        self, StringSplitIter(context_str, ','), std::make_index_sequence<NUM_FIELDS>{}
    ); res.is_err()) return Err(res.unwrap_err());

    return Ok(self);
}

}