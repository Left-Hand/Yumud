#include "src/testbench/tb.h"

#include "sys/debug/debug.hpp"
#include <cxxabi.h>
#include <new>
#include <chrono>

// #define HEAP_TB_ENABLED



// String addr_to_symbol(void * addr){
    // char ** strings = backtrace_symbols(&addr, 1); 
    // if(strings == nullptr){
    //     return String("<unknown>", true);
    // }
    
    // String ret = strings[0];
    // free(strings);
    
    // if(ret.find("(") == String::npos){
    //     return ret;
    // }
    
//     return String(abi::__cxa_demangle(abi::__cxa_current_exception_type()->name(), nullptr, nullptr, nullptr), true);
// }

#ifdef HEAP_TB_ENABLED

void *operator new(size_t size){
    void * caller = __builtin_return_address(0);
    void * ptr = malloc(size);
    if(unlikely(ptr == nullptr)){
        PANIC("Out of memory", addt_to_symbol(caller));
    }

    return ptr;
}

void operator delete(void * ptr){
    free(ptr);
}


#endif


void heap_main(){
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
}