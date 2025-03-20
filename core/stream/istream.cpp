#include "istream.hpp"

using namespace ymd;

// String InputStream::readString(const size_t len){
//     String str;
//     str.reserve(len + 1);

//     for(uint8_t i = 0; i < len; i++){
//         char _;
//         read(_);
//         str.concat(_);
//     }

//     return str;
// }

// String InputStream::readStringUntil(const char & chr) {
//     String str;
//     size_t cap = 16;
//     str.reserve(cap);

//     char _;
//     while (true) {
//         read(_);

//         if (_ == chr) {
//             break;
//         }

//         str.concat(_);

//         if ((size_t)str.length() == cap) {
//             cap *= 2;
//             str.reserve(cap);
//         }
//     }

//     return str;
// }