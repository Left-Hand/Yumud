#pragma once


#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "primitive/arithmetic/angle.hpp"

#include "hal/bus/spi/spidrv.hpp"
#include "tcan1145_prelude.hpp"

namespace ymd::drivers{


struct TCAN114X:public TCAN1145_Prelude{


    /* ************************************ *
    *           Device Functions           *
    * ************************************ */
    Mode Mode_Read();
    IResult<> Mode_Set(Mode modeRequest);

    /* ************************************* *
    *             Helper Functions          *
    * ************************************* */
    IResult<> Mode_Control_Read(ModeControl *ModeControl);
    IResult<> Mode_Control_Write(ModeControl ModeControl);
    IResult<> Wake_Pin_Config_Read(Wake_Pin_Config *WakePin);
    IResult<> Wake_Pin_Config_Write(Wake_Pin_Config WakePin);
    IResult<> Device_Pin_Config_Read(Pin_Config *PinConfig);
    IResult<> Device_Pin_Config_Write(Pin_Config PinConfig);
    IResult<> PN_Config_Read(PN_Config *pnConfig);
    IResult<> PN_Config_Write(PN_Config pnConfig);

    IResult<> Device_Interrupts_Read(Interrupts *ir);
    IResult<> Device_Interrupts_Clear(Interrupts *ir);
    IResult<> Device_Interrupts_ClearAll();
    IResult<Dev_ID> Device_ID_Read();


    IResult<> WDT_Config_Write(const WatchdogConfig & WDTConfig);
    IResult<WatchdogConfig> WDT_Config_Read();
    IResult<> WDT_Config_QA_Write(const Watchdog_QA_Config & QAConfig);
    IResult<Watchdog_QA_Config> WDT_Config_QA_Read();
    IResult<> WDT_Disable();

    IResult<> WDT_Reset();
    IResult<> WDT_QA_Read_Question(uint8_t *question);
    IResult<> WDT_QA_Write_Answer(uint8_t answer);

private:


    #if 0
    uint8_t write_reg(uint8_t address, uint8_t *data)
    {
        uint16_t out        = 0;
        uint8_t intRegister = 0;
        uint16_t dataAddr   = 0;

        dataAddr = ((((uint16_t) address << 1) | 0x01) << 8);  // 1 for write
        dataAddr |= *data;

        DL_SPI_transmitDataBlocking16(SPI_0_INST, (dataAddr));
        while (DL_SPI_isBusy(SPI_0_INST) == true)
            ;

        DL_SPI_receiveDataCheck16(SPI_0_INST, &out);

        intRegister = (uint8_t)(out & 0xFF);
        return intRegister;
    }
    uint8_t read_reg(uint8_t address, uint8_t *data)
    {
        uint16_t out        = 0;
        uint8_t intRegister = 0;

        DL_SPI_transmitDataBlocking16(SPI_0_INST, (((uint16_t) address) << 9U));
        while (DL_SPI_isBusy(SPI_0_INST) == true)
            ;

        DL_SPI_receiveDataCheck16(SPI_0_INST, &out);
        intRegister = (uint8_t)((out & 0xFF00U) >> 8U);

        *data = (uint8_t)(out & 0x00FFU);
        return intRegister;
    }
    #endif

    IResult<> write_reg(uint8_t address, uint8_t data){
        return Ok();
    }
    IResult<> read_reg(uint8_t address, uint8_t *data){
        return Ok();
    }

};



}