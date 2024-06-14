#include "testbench/tb.h"

int main(){
    Sys::Misc::prework();

    // stepper_app();
    // stepper_app_new();
    // StepperTest::stepper_test();

    // image.putseg_h8_unsafe(Vector2i{0,0}, 0x5a, true);
    // image.putseg_v8_unsafe(Vector2i{0,0}, 0x39, true);

    // for(auto & datum : data){
    //     logger.println(datum);
    // }
    Stepper stp;
    uart1.init(115200 * 8);
    can1.init(Can::BaudRate::Mbps1);

    stp.init();
    stp.setCurrentClamp(0.3);
    while(true){
        stp.run();
        stp.setTargetPosition(8*sin(t));

        stp.setTargetSpeed(30 + 5 * sin(t));
    }
    // if(false){
    //     constexpr int page_size = 8;
    //     Rangei plat = {11, 17};
    //     Rangei window = {0, 0};

    //     AT24C02_DEBUG(window = plat.grid_forward(window, page_size));
    //     AT24C02_DEBUG(window = plat.grid_forward(window, page_size));
    //     AT24C02_DEBUG(window = plat.grid_forward(window, page_size));
    //     AT24C02_DEBUG(window = plat.grid_forward(window, page_size));
    //     // AT24C02_DEBUG(plat.gird_part(17, page_size, false));
    // }

    // modem_app();
    // test_app();
    // pmdc_test();
    // SpreadCycle::chopper_test();
    // buck_test();
    // osc_test();
}