#ifndef __CANSF_SEMANTICS_HPP__

#define __CANSF_SEMANTICS_HPP__

#include <cstdint>
#include <types/string/String.hpp>
namespace CANSF{
    class Semantics{
        protected:
            struct Binding{
                void * ptr;
                uint8_t size;
            };

            Binding bindings[8] = {{.ptr = nullptr, .size = 0}};

            const String & header;
        public:
            template<T>
            void bind(T * data, uint8_t index){
                bindings[index].ptr = data;
                bindings[index].size = sizeof(T);
            }

            template <typename real, typename... Args>
            void println(real first, Args... args) {
                *this << first;
                if(!skipSpec) *this << space;
                else skipSpec = false;
                println(args...);
            }
    };
};

#endif