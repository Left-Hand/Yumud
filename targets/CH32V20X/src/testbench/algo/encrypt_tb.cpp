#include <bitset>
#include <ranges>


#include "src/testbench/tb.h"

#include "core/debug/debug.hpp"
#include "core/math/real.hpp"
#include "core/math/realmath.hpp"
#include "core/utils/Result.hpp"

#include "hal/conn/uart/hw_singleton.hpp"
#include "algo/encrypt/base64.hpp"
#include "algo/encrypt/sha256.hpp"
#include "algo/encrypt/aes.hpp"
#include "algo/encrypt/lz77.hpp"
#include "algo/encrypt/curve25519.hpp"

using namespace ymd;
using namespace ymd::encrypt;

#define UART hal::uart6
static constexpr size_t UART_BAUD = 576000u;

void encrypt_main(){
    auto init_debugger = []{
        auto & DBG_UART = DEBUGGER_INST;

        DBG_UART.init({
            .remap = hal::UartRemap::_0,
            .baudrate = hal::NearestFreq(UART_BAUD),
        });

        DEBUGGER.retarget(&DBG_UART);
        DEBUGGER.build_config()
            .set_eps(4)
            .set_splitter(",")
            .no_brackets(EN)
            .no_fieldname(EN)
            .force_sync(EN)
            .finalize();
    };

    init_debugger();


}