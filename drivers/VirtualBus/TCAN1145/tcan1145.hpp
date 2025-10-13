#pragma once


#include "core/io/regs.hpp"
#include "core/utils/Result.hpp"
#include "core/utils/angle.hpp"

#include "hal/bus/spi/spidrv.hpp"
#include "tcan1145_prelude.hpp"

namespace ymd::drivers{


struct TCAN114X:public TCAN1145_Prelude{


    /* ************************************ *
    *           Device Functions           *
    * ************************************ */
    Mode Mode_Read(void);
    IResult<> Mode_Set(Mode modeRequest);

    /* ************************************* *
    *             Helper Functions          *
    * ************************************* */
    IResult<> Mode_Control_Read(ModeControl *ModeControl);
    IResult<> Mode_Control_Write(ModeControl *ModeControl);
    IResult<> Wake_Pin_Config_Read(
        Wake_Pin_Config *WakePin);
    IResult<> Wake_Pin_Config_Write(
        Wake_Pin_Config *WakePin);
    IResult<> Device_Pin_Config_Read(
        Pin_Config *PinConfig);
    IResult<> Device_Pin_Config_Write(
        Pin_Config *PinConfig);
    IResult<> PN_Config_Read(PN_Config *pnConfig);
    IResult<> PN_Config_Write(PN_Config *pnConfig);

    IResult<> Device_Interrupts_Read(Interrupts *ir);
    IResult<> Device_Interrupts_Clear(Interrupts *ir);
    IResult<> Device_Interrupts_ClearAll(void);
    IResult<> Device_ID_Read(Dev_ID *devID);



    /*
    * Get and Set Methods
    */
    IResult<> WDT_Config_Write(Watchdog_Config *WDTConfig);
    IResult<> WDT_Config_Read(Watchdog_Config *WDTConfig);
    IResult<> WDT_Config_QA_Write(Watchdog_QA_Config *QAConfig);
    IResult<> WDT_Config_QA_Read(Watchdog_QA_Config *QAConfig);
    IResult<> WDT_Disable(void);

    /*
    * Functions
    */
    IResult<> WDT_Reset(void);
    IResult<> WDT_QA_Read_Question(uint8_t *question);
    IResult<> WDT_QA_Write_Answer(uint8_t *answer);
    void WDT_QA_Calculate_Answer(
        uint8_t *question, uint8_t answer[], uint8_t answerConfig);

    void WDT_QA_Generate_Question_Bit_Array(
        uint8_t config, wdt_bits *bitArray);
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

    /*
    * @brief Calculates the next 4 bit question
    * @param *question is a pointer to the question used to calculate the answers
    * @param answer[] is an array of at least 4 bytes used to store the calculated answers
    */
    static constexpr void WDT_QA_Calculate_Question(uint8_t *question, uint8_t answer[])
    {
        uint8_t crc = 0;
        uint8_t i, j;
        uint8_t CRC7_POLY = 0x19;

        for (i = 0; i < 4; i++) {
            crc ^= *question;
            for (j = 0; j < 8; j++) {
                if (crc & 1) crc ^= CRC7_POLY;
                crc >>= 1;
            }
            answer[i] = crc;
        }
    }

};



}