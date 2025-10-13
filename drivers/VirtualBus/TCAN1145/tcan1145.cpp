#include "tcan1145.hpp"

using namespace ymd;
using namespace ymd::drivers;

using Error = TCAN114X::Error;

template<typename T = void>
using IResult = Result<T, Error>;


// Wake Pin Pulse Direction: Low -> High -> Low
#define TCAN114X_PIN_CONFIG_WAKE_PULSE_LHL 0
// Wake Pin Pulse Direction: High -> Low -> High
#define TCAN114X_PIN_CONFIG_WAKE_PULSE_HLH 1

#define TCAN114X_PIN_CONFIG_SDO_ONLY 0
#define TCAN114X_PIN_CONFIG_SDO_AND_NINT 1

#define TCAN114X_PIN_CONFIG_NINT_SEL_GLOBAL_INT 0
#define TCAN114X_PIN_CONFIG_NINT_SEL_WDT_FAILURE 1
#define TCAN114X_PIN_CONFIG_NINT_SEL_BUS_FAULT 2
#define TCAN114X_PIN_CONFIG_NINT_SEL_WAKE_REQUEST 3

#define TCAN114X_PIN_CONFIG_RXD_WK_PULLED_LOW 0
#define TCAN114X_PIN_CONFIG_RXD_WK_TOGGLE 1

#define TCAN114X_PIN_CONFIG_GPO_SEL_WDT_FAILURE 0
#define TCAN114X_PIN_CONFIG_GPO_SEL_GLOBAL_INT 1
#define TCAN114X_PIN_CONFIG_GPO_SEL_BUS_FAULT 2
#define TCAN114X_PIN_CONFIG_GPO_SEL_WAKE_REQUEST 3


#define TCAN114X_WDT_CONFIG_WD_CONFIG_MODE_DISABLED 0
#define TCAN114X_WDT_CONFIG_WD_CONFIG_MODE_TIMEOUT 1
#define TCAN114X_WDT_CONFIG_WD_CONFIG_MODE_WINDOW 2
#define TCAN114X_WDT_CONFIG_WD_CONFIG_MODE_QA 3

#define TCAN114X_WDT_CONFIG_WD_PRE_FACTOR_1 0
#define TCAN114X_WDT_CONFIG_WD_PRE_FACTOR_2 1
#define TCAN114X_WDT_CONFIG_WD_PRE_FACTOR_3 2
#define TCAN114X_WDT_CONFIG_WD_PRE_FACTOR_4 3

#define TCAN114X_WDT_CONFIG_WD_ERR_CNT_IMMEDIATE 0
#define TCAN114X_WDT_CONFIG_WD_ERR_CNT_5TH_EVENT 1
#define TCAN114X_WDT_CONFIG_WD_ERR_CNT_9TH_EVENT 2
#define TCAN114X_WDT_CONFIG_WD_ERR_CNT_15TH_EVENT 3

#define TCAN114X_WDT_CONFIG_WD_ACT_INH_TOGGLE 0
#define TCAN114X_WDT_CONFIG_WD_ACT_GPO_LOW 1
#define TCAN114X_WDT_CONFIG_WD_ACT_WD_INT 2

#define TCAN114X_WDT_CONFIG_RESET_PULSE_WIDTH_1MS 8
#define TCAN114X_WDT_CONFIG_RESET_PULSE_WIDTH_5MS 1
#define TCAN114X_WDT_CONFIG_RESET_PULSE_WIDTH_12MS 2
#define TCAN114X_WDT_CONFIG_RESET_PULSE_WIDTH_25MS 11
#define TCAN114X_WDT_CONFIG_RESET_PULSE_WIDTH_50MS 4
#define TCAN114X_WDT_CONFIG_RESET_PULSE_WIDTH_75MS 13
#define TCAN114X_WDT_CONFIG_RESET_PULSE_WIDTH_125MS 14
#define TCAN114X_WDT_CONFIG_RESET_PULSE_WIDTH_190MS 7

#define TCAN114X_SLEEP 1
#define TCAN114X_STANDBY 4
#define TCAN114X_LISTEN 5
#define TCAN114X_NORMAL 7



#define TCAN114X_WAKE_CONFIG_BIDIRECTIONAL 0
#define TCAN114X_WAKE_CONFIG_RISING_EDGE 1
#define TCAN114X_WAKE_CONFIG_FALLING_EDGE 2
#define TCAN114X_WAKE_CONFIG_PULSE 3

#define TCAN114X_WAKE_WIDTH_MIN_10MS 0
#define TCAN114X_WAKE_WIDTH_MIN_20MS 1
#define TCAN114X_WAKE_WIDTH_MIN_40MS 2
#define TCAN114X_WAKE_WIDTH_MIN_80MS 3

#define TCAN114X_WAKE_WIDTH_INVALID_5MS 0
#define TCAN114X_WAKE_WIDTH_INVALID_10MS 1
#define TCAN114X_WAKE_WIDTH_INVALID_20MS 2
#define TCAN114X_WAKE_WIDTH_INVALID_40MS 3

#define TCAN114X_WAKE_WIDTH_MAX_750MS 0
#define TCAN114X_WAKE_WIDTH_MAX_1000MS 1
#define TCAN114X_WAKE_WIDTH_MAX_1500MS 2
#define TCAN114X_WAKE_WIDTH_MAX_2000MS 3

/*
 * @brief Configures the watchdog registers with the settings from the passed struct
 * @param *WDTConfig is a pointer to a @c Watchdog_Config struct
 * @returns IResult<> if the write was successful or not, based on if TCAN114X_CONFIGURE_VERIFY_WRITES is defined in tcan114x.h
 */
IResult<>
TCAN114X::WDT_Config_Write(Watchdog_Config *WDTConfig)
{
    // Step one is to write the words to the registers
    if(const auto res = write_reg(REG_TCAN114X_WD_CONFIG_1, WDTConfig->word_config_1);
        res.is_err()) return Err(res.unwrap_err());


    // Write the second word to the register
    if(const auto res = write_reg(REG_TCAN114X_WD_CONFIG_2, WDTConfig->word_config_2);
        res.is_err()) return Err(res.unwrap_err());

    // Write the third word to the register
    if(const auto res = write_reg(REG_TCAN114X_WD_RST_PULSE, WDTConfig->word_wd_rst_pulse);
        res.is_err()) return Err(res.unwrap_err());


    return Ok();
}

/*
 * @brief Reads the current configuration of the watchdog and updates the passed struct
 * @param *WDTConfig is a pointer to a @c Watchdog_Config struct
 * @returns IResult<> if the write was successful or not, based on if TCAN114X_CONFIGURE_VERIFY_WRITES is defined in tcan114x.h
 */
IResult<>
TCAN114X::WDT_Config_Read(Watchdog_Config *WDTConfig)
{
    uint8_t readWDT;
    if(const auto res = read_reg(REG_TCAN114X_WD_CONFIG_1, &readWDT);
        res.is_err()) return Err(res.unwrap_err());
    WDTConfig->word_config_1 = readWDT;

    if(const auto res = read_reg(REG_TCAN114X_WD_CONFIG_2, &readWDT);
        res.is_err()) return Err(res.unwrap_err());
    WDTConfig->word_config_2 = readWDT;

    if(const auto res = read_reg(REG_TCAN114X_WD_RST_PULSE, &readWDT);
        res.is_err()) return Err(res.unwrap_err());
    WDTConfig->word_wd_rst_pulse = readWDT;

    return Ok();
}

/*
 * @brief Configures the QA watchdog register with the settings from the passed struct
 * @param *QAConfig is a pointer to a @c Watchdog_QA_Config struct
 * @returns IResult<> if the write was successful or not, based on if TCAN114X_CONFIGURE_VERIFY_WRITES is defined in tcan114x.h
 */
IResult<>
TCAN114X::WDT_Config_QA_Write(Watchdog_QA_Config *QAConfig)
{
    if(const auto res = write_reg(REG_TCAN114X_WD_QA_CONFIG, QAConfig->word_qa_config);
        res.is_err()) return Err(res.unwrap_err());

    return Ok();
}

/*
 * @brief Reads the current configuration of the QA watchdog and updates the passed struct
 * @param *QAConfig is a pointer to a @c Watchdog_QA_Config struct
 * @returns IResult<> if the write was successful or not, based on if TCAN114X_CONFIGURE_VERIFY_WRITES is defined in tcan114x.h
 */
IResult<>
TCAN114X::WDT_Config_QA_Read(Watchdog_QA_Config *QAConfig)
{
    uint8_t readWDT;
    if(const auto res = read_reg(REG_TCAN114X_WD_QA_CONFIG, &readWDT);
        res.is_err()) return Err(res.unwrap_err());
    QAConfig->word_qa_config = readWDT;
    return Ok();
}

/*
 * @briefe Disables the watchdog, it must be re-configured to be enabled
 */
IResult<>
TCAN114X::WDT_Disable(void)
{
    uint8_t data;
    if(const auto res = read_reg(REG_TCAN114X_WD_CONFIG_1, &data);
        res.is_err()) return res;
    data &= ~REG_BITS_TCAN114X_WD_CONFIG_1_WD_CONFIG_MASK;
    if(const auto res = write_reg(REG_TCAN114X_WD_CONFIG_1, data);
        res.is_err()) return res;

    return Ok();
}

/*
 * @brief Writes to the WD timer reset register to perform a reset
 */
IResult<> TCAN114X::WDT_Reset()
{
    // Write 0xFF to the input reset register to reset the watchdog
    uint8_t data = 0xFF;
    if(const auto res = write_reg(REG_TCAN114X_WD_INPUT_TRIG, data);
        res.is_err()) return res;
    return Ok();
}

/*
 * @brief Reads the current question from the TCAN114X and writes it to the question pointer
 * @param *question is a pointer where the question will be stored
 */
IResult<> TCAN114X::WDT_QA_Read_Question(uint8_t *question)
{
    uint8_t read;
    if(const auto res = read_reg(REG_TCAN114X_WD_QA_QUESTION, &read);
        res.is_err()) return Err(res.unwrap_err());
    *question = read & 0x0F;
    return Ok();
}

/*
 * @brief Writes the supplied answer to the TCAN114X
 * @param *answer is a pointer to the answer that will be written to the TCAN114X
 */
IResult<> TCAN114X::WDT_QA_Write_Answer(uint8_t *answer)
{
    if(const auto res = write_reg(REG_TCAN114X_WD_QA_ANSWER, *answer);
        res.is_err()) return res;
    return Ok();
}


static constexpr uint8_t WDT_QA_Get_Bit(uint8_t data, uint8_t bit){
    if (bit > 3) return 0;
    return ((data >> bit) & 0x01);
}

static constexpr uint8_t WDT_QA_Return_XOR(uint8_t data)
{
    if (data % 2) return 1;
    return 0;
}


/*
 * @brief Calculates the 4 answers needed based on the supplied question
 * @param *question is a pointer to the question used to calculate the answers
 * @param answer[] is an array of at least 4 bytes used to store the calculated answers
 */
void TCAN114X::WDT_QA_Calculate_Answer(
    uint8_t *question, uint8_t answer[], uint8_t answerConfig)
{
    // We'll need a way to map the configuration mux to the array of bits.
    wdt_bits bitArray = {0};
    WDT_QA_Generate_Question_Bit_Array(answerConfig, &bitArray);

    // Gets us the current WD_ANS_CNT value (starts at 3, and goes to 0
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t wd_cnt = 3 - i;

        uint8_t bitanswer = 0;
        uint8_t temp;

        // Bit 0
        temp = WDT_QA_Get_Bit(wd_cnt, 1) ^
                WDT_QA_Get_Bit(*question, bitArray.wd1);
        temp ^= WDT_QA_Get_Bit(*question, bitArray.wd0);
        bitanswer = temp;

        // Bit 1
        temp = WDT_QA_Get_Bit(*question, 1) ^
                WDT_QA_Get_Bit(*question, bitArray.wd3);
        temp += WDT_QA_Get_Bit(wd_cnt, 1);
        temp += WDT_QA_Get_Bit(*question, bitArray.wd2);
        bitanswer |= (WDT_QA_Return_XOR(temp) << 1);

        // Bit 2
        temp = WDT_QA_Get_Bit(*question, 1) ^
                WDT_QA_Get_Bit(*question, bitArray.wd5);
        temp += WDT_QA_Get_Bit(wd_cnt, 1);
        temp += WDT_QA_Get_Bit(*question, bitArray.wd4);
        bitanswer |= (WDT_QA_Return_XOR(temp) << 2);

        // Bit 3
        temp = WDT_QA_Get_Bit(*question, 3) ^
                WDT_QA_Get_Bit(*question, bitArray.wd7);
        temp += WDT_QA_Get_Bit(wd_cnt, 1);
        temp += WDT_QA_Get_Bit(*question, bitArray.wd6);
        bitanswer |= (WDT_QA_Return_XOR(temp) << 3);

        // Bit 4
        temp = WDT_QA_Get_Bit(wd_cnt, 0) ^
                WDT_QA_Get_Bit(*question, bitArray.wd8);
        bitanswer |= (temp << 4);

        // Bit 5
        temp = WDT_QA_Get_Bit(wd_cnt, 0) ^
                WDT_QA_Get_Bit(*question, bitArray.wd9);
        bitanswer |= (temp << 5);

        // Bit 6
        temp = WDT_QA_Get_Bit(wd_cnt, 0) ^
                WDT_QA_Get_Bit(*question, bitArray.wd10);
        bitanswer |= (temp << 6);

        // Bit 7
        temp = WDT_QA_Get_Bit(wd_cnt, 0) ^
                WDT_QA_Get_Bit(*question, bitArray.wd11);
        bitanswer |= (temp << 7);

        answer[i] = bitanswer;
    }
}


void TCAN114X::WDT_QA_Generate_Question_Bit_Array(
    uint8_t config, wdt_bits *bitArray)
{
    switch (config) {
        default:
            return;
        case 0x00: {
            bitArray->wd0  = 0;
            bitArray->wd1  = 3;
            bitArray->wd2  = 0;
            bitArray->wd3  = 2;
            bitArray->wd4  = 0;
            bitArray->wd5  = 3;
            bitArray->wd6  = 2;
            bitArray->wd7  = 0;
            bitArray->wd8  = 1;
            bitArray->wd9  = 3;
            bitArray->wd10 = 0;
            bitArray->wd11 = 2;
            break;
        }
        case 0x01: {
            bitArray->wd0  = 1;
            bitArray->wd1  = 2;
            bitArray->wd2  = 1;
            bitArray->wd3  = 1;
            bitArray->wd4  = 3;
            bitArray->wd5  = 2;
            bitArray->wd6  = 1;
            bitArray->wd7  = 3;
            bitArray->wd8  = 0;
            bitArray->wd9  = 2;
            bitArray->wd10 = 3;
            bitArray->wd11 = 1;
            break;
        }
        case 0x02: {
            bitArray->wd0  = 2;
            bitArray->wd1  = 1;
            bitArray->wd2  = 2;
            bitArray->wd3  = 0;
            bitArray->wd4  = 1;
            bitArray->wd5  = 1;
            bitArray->wd6  = 0;
            bitArray->wd7  = 2;
            bitArray->wd8  = 2;
            bitArray->wd9  = 1;
            bitArray->wd10 = 2;
            bitArray->wd11 = 0;
            break;
        }
        case 0x03: {
            bitArray->wd0  = 3;
            bitArray->wd1  = 0;
            bitArray->wd2  = 3;
            bitArray->wd3  = 3;
            bitArray->wd4  = 1;
            bitArray->wd5  = 0;
            bitArray->wd6  = 3;
            bitArray->wd7  = 1;
            bitArray->wd8  = 3;
            bitArray->wd9  = 0;
            bitArray->wd10 = 1;
            bitArray->wd11 = 3;
            break;
        }
    }
}

