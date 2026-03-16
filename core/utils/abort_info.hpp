#pragma once

#include <cstdint>
#include <source_location>
#include <bit>
#include "core/tmp/reflect/type_name.hpp"

namespace ymd{

// template<typename T>
// struct Option;

// template<typename T, typename E>
// struct Result;

struct [[nodiscard]] alignas(4) AbortInfo final{
    using Self = AbortInfo;

    const char* file_name;
    const char* function_name;
    uint32_t line;
    uint32_t column;

    const char* reason;


    struct [[nodiscard]] alignas(4) Arguments final{
        struct alignas(4) [[nodiscard]] ArgDescr final{
            enum class [[nodiscard]] Kind:uint32_t{
                Empty,
                Cstring,
            };

            Kind kind;

            static constexpr ArgDescr from_cstr(){
                return ArgDescr{
                    .kind = Kind::Cstring,
                };
            }

            static constexpr ArgDescr from_empty(){
                return ArgDescr{
                    .kind = Kind::Empty
                };
            }

            [[nodiscard]] constexpr bool is_cstr() const{
                return kind == Kind::Cstring;
            }

            [[nodiscard]] constexpr uint32_t to_u32() const{
                return std::bit_cast<uint32_t>(*this);
            }
        };

    
        struct [[nodiscard]] alignas(4) Flag final{
            uint32_t arg1_descr_bits:10;
            uint32_t arg2_descr_bits:10;

            uint32_t is_option:1;
            uint32_t is_result:1;

            static constexpr Flag zero(){
                return std::bit_cast<Flag>(uint32_t(0));
            }

            [[nodiscard]] constexpr uint32_t to_u32() const{
                return std::bit_cast<uint32_t>(*this);
            }

            template<size_t I>
            constexpr ArgDescr arg_descr() const{
                static_assert(I <= 2);
                static_assert(I != 0);

                if constexpr(I == 1){
                    return std::bit_cast<ArgDescr>(this->arg1_descr_bits);
                }else if constexpr(I == 2){
                    return std::bit_cast<ArgDescr>(this->arg2_descr_bits);
                }
            }
        };
    
        Flag flag;
        const void * arg1;
        const void * arg2;

        static constexpr Arguments from_default(){
            return Arguments{
                .flag = Flag::zero(),
                .arg1 = nullptr,
                .arg2 = nullptr,
            };
        }

        template<typename T>
        static constexpr Arguments from_option(){
            Flag flag = Flag::zero();
            flag.arg1_descr_bits = ArgDescr::from_cstr().to_u32();
            flag.arg2_descr_bits = ArgDescr::from_empty().to_u32();
            flag.is_option = 1;
            return Arguments{
                .flag = flag,
                .arg1 = tmp::type_name_v<T>.data(),
                .arg2 = nullptr,
            };
        }

        template<typename T, typename E>
        static constexpr Arguments from_result(){ 
            Flag flag = Flag::zero();
            flag.arg1_descr_bits = ArgDescr::from_cstr().to_u32();
            flag.arg2_descr_bits = ArgDescr::from_cstr().to_u32();
            flag.is_result = 1;
            return Arguments{
                .flag = flag,
                .arg1 = tmp::type_name_v<T>.data(),
                .arg2 = tmp::type_name_v<E>.data(),
            };
        }
    };

    Arguments arguments;
    static constexpr Self from_default(){
        return Self{
            .file_name = nullptr, 
            .function_name = nullptr, 
            .line = 0, 
            .column = 0, 
            .reason = nullptr, 
            .arguments = Arguments::from_default()
        };
    }

    static constexpr Self from_reason(
        const char* reason, 
        const Arguments arguments = Arguments::from_default()
    ){
        return Self{
            .file_name = nullptr, 
            .function_name = nullptr, 
            .line = 0, 
            .column = 0, 
            .reason = reason,
            .arguments = arguments
        };
    }
    static constexpr Self with_location(
        const char* reason, 
        const Arguments arguments,
        const std::source_location location
    ){
        return Self{
            .file_name = location.file_name(), 
            .function_name = location.function_name(),
            .line = location.line(),
            .column = location.column(),
            .reason = reason,
            .arguments = arguments
        };
    }

    static constexpr Self without_arguments(
        const char* reason, 
        const std::source_location location
    ){
        return Self::with_location(
            reason,
            AbortInfo::Arguments::from_default(),
            location
        );
    }
};

static_assert(sizeof(AbortInfo) == 4 * 8);
}