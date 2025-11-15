
// https://github.com/umeiko/rpi2040-heimann-htpa32x32d-touchscreen/blob/main/main/src/probe/heimann_driver.hpp

/*** PROGRAMM INFO***************************************************************************************
  source code for HTPAd RP2040 driver
  name:           RP2040_HTPAd_32x32
  version/date:   2.2-rp2040branch / 01 Jan 2025
  programmer:     Umeko (umeko@stu.xmu.edu.cn)
*********************************************************************************************************/

/*** PROGRAMM INFO***************************************************************************************
  source code for ESP32 and HTPAd Application Shield
  name:           ESP32_HTPAd_32x32.ino
  version/date:   2.2 / 20 Dec 2022
  programmer:     Heimann Sensor GmbH / written by Dennis Pauer (pauer@heimannsensor.com)
*********************************************************************************************************/

#pragma once


#include "htpa32x32_constants.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"

namespace ymd::drivers::htpa32x32{
static constexpr size_t I2C_BUFFER_LENGTH = 300;
struct Characteristics {
    uint16_t NumberOfPixel;
    uint8_t NumberOfBlocks;
    uint8_t RowPerBlock;
    uint16_t PixelPerBlock;
    uint16_t PixelPerColumn;
    uint16_t PixelPerRow;
    uint8_t AllowedDeadPix;
    uint16_t TableNumber;
    uint16_t TableOffset;
    uint8_t PTATPos;
    uint8_t VDDPos;
    uint8_t PTATVDDSwitch;
    uint8_t CyclopsActive;
    uint8_t CyclopsPos;
    uint8_t DataPos;
};

Characteristics DevConst = {
    NUMBER_OF_PIXEL,
    NUMBER_OF_BLOCKS,
    ROW_PER_BLOCK,
    PIXEL_PER_BLOCK,
    PIXEL_PER_COLUMN,
    PIXEL_PER_ROW,
    ALLOWED_DEADPIX,
    TABLENUMBER,
    TABLEOFFSET,
    PTAT_POS,
    VDD_POS,
    PTAT_VDD_SWITCH,
    ATC_ACTIVE,
    ATC_POS,
    DATA_POS,
};

struct HTPA32x32{
//-----------------------------------------
// EEPROM DATA
uint8_t mbit_calib, bias_calib, clk_calib, bpa_calib, pu_calib, mbit_user, bias_user, clk_user, bpa_user, pu_user;
uint8_t nrofdefpix, gradscale, vddscgrad, vddscoff, epsilon, lastepsilon, arraytype;
uint8_t deadpixmask[ALLOWED_DEADPIX];
int8_t globaloff;
int16_t thgrad[PIXEL_PER_COLUMN][PIXEL_PER_ROW];
uint16_t tablenumber, vddth1, vddth2, ptatth1, ptatth2, ptatgr, globalgain;
uint16_t deadpixadr[ALLOWED_DEADPIX * 2];
int16_t thoffset[PIXEL_PER_COLUMN][PIXEL_PER_ROW];
int16_t vddcompgrad[ROW_PER_BLOCK * 2][PIXEL_PER_ROW];
int16_t vddcompoff[ROW_PER_BLOCK * 2][PIXEL_PER_ROW];
uint32_t id, ptatoff;
float ptatgr_float, ptatoff_float, pixcmin, pixcmax, bw;
uint32_t *pixc2_0; // start address of the allocated heap memory
uint32_t *pixc2; // increasing address pointer

//-----------------------------------------
// SENSOR DATA
uint16_t data_pixel[PIXEL_PER_COLUMN][PIXEL_PER_ROW];
uint16_t* data_pixel_flatten = (uint16_t*)data_pixel;
uint8_t RAMoutput[2 * NUMBER_OF_BLOCKS + 2][BLOCK_LENGTH];
bool flag_min_max_initaled = false;  // 需要重新计算极值的标志位
uint8_t x_max, y_max, x_min, y_min;


/*
RAMoutput is the place where the raw values are saved

example, order for 80x64:
RAMoutput[0][]... data from block 0 top
RAMoutput[1][]... data from block 1 top
RAMoutput[2][]... data from block 2 top
RAMoutput[3][]... data from block 3 top
RAMutput[4][]... electrical offset top
RAMoutput[5][]... electrical offset bottom
RAMoutput[6][]... data from block 3 bottom
RAMoutput[7][]... data from block 2 bottom
RAMoutput[8][]... data from block 1 bottom
RAMoutput[9][]... data from block 0 bottom

*/
uint16_t eloffset[ROW_PER_BLOCK * 2][PIXEL_PER_ROW];
uint8_t statusreg;
uint16_t Ta, ptat_av_uint16, vdd_av_uint16;


// BUFFER for PTAT,VDD and elOffsets
// PTAT:
uint16_t ptat_buffer[PTAT_BUFFER_SIZE];
uint16_t ptat_buffer_average;
uint8_t use_ptat_buffer = 0;
uint8_t ptat_i = 0;
uint8_t PTATok = 0;
// VDD:
uint16_t vdd_buffer[VDD_BUFFER_SIZE];
uint16_t vdd_buffer_average;
uint8_t use_vdd_buffer = 0;
uint8_t vdd_i = 0;
// electrical offsets:
uint8_t use_eloffsets_buffer = 0;
uint8_t eloffsets_i = 0;
uint8_t new_offsets = 1;

// PROGRAMM CONTROL
bool switch_ptat_vdd = 0;
uint8_t adr_offset = 0x00;
uint8_t send_data = 0;
uint16_t picnum = 0;
uint8_t state = 0;
uint8_t read_block_num = START_WITH_BLOCK; // start with electrical offset
uint8_t read_eloffset_next_pic = 0;
uint8_t gui_mode = 0;
uint8_t wait_pic = 0;
bool ReadingRoutineEnable = 1;

// OTHER
uint32_t gradscale_div;
uint32_t vddscgrad_div;
uint32_t vddscoff_div;
int vddcompgrad_n;
int vddcompoff_n;
uint32_t t1;
uint8_t print_state = 0;


unsigned NewDataAvailable = 1;

uint16_t timert;
char serial_input = 'm';
hal::I2cDrv i2c_drv_;

/********************************************************************
 ********************************************************************
    - - - PART 2: HTPAd FUNCTIONS - - -
    calcPixC()
    calculate_pixel_temp()
    pixel_masking()
    readblockinterrupt()
    read_eeprom()
    read_EEPROM_byte( uint8_t addr)
    read_sensor_register()
    sort_data()
    write_calibration_settings_to_sensor()
    write_sensor_byte( uint8_t addr, uint8_t reg_val)
 ********************************************************************
 ********************************************************************/

/********************************************************************
   Function:        void read_EEPROM_byte(uint32_t eeaddress )
   Description:     read eeprom register as 8
   Dependencies:    register address (address)
 *******************************************************************/
uint8_t read_EEPROM_byte(uint16_t reg_addr) {
    uint8_t ret;
    i2c_drv_.read_reg(reg_addr, ret, std::endian::big);
}

/********************************************************************
   Function:        void read_EEPROM_byte(uint32_t eeaddress )
   Description:     read eeprom register as 8
   Dependencies:    register address (address)
 *******************************************************************/
void write_EEPROM_byte(uint16_t reg_addr, uint8_t content ) {
    i2c_drv_.write_reg(reg_addr, content, std::endian::big);
}

/********************************************************************
   Function:        void write_sensor_byte( uint16_t addr)
   Description:     write to sensor register
   Dependencies:    register address (addr),
                    number of bytes (n)
 *******************************************************************/
void write_sensor_byte(uint8_t reg_addr, uint8_t reg_val) {
    i2c_drv_.write_reg(reg_addr, reg_val);
}

/********************************************************************
   Function:        void read_sensor_register( uint16_t addr, uint8_t *dest, uint16_t n)
   Description:     read sensor register
 *******************************************************************/

void read_sensor_register(uint16_t addr, uint8_t *dest, uint16_t n)
{
    i2c_drv_.read_burst(addr, std::span<uint8_t>(dest, n));
}

/********************************************************************
   Function:        void pixel_masking()
   Description:     repair dead pixel by using the average of the neighbors
 *******************************************************************/
void pixel_masking() {


    uint8_t number_neighbours[ALLOWED_DEADPIX];
    uint32_t temp_defpix[ALLOWED_DEADPIX];
    for (int i = 0; i < nrofdefpix; i++) {
        number_neighbours[i] = 0;
        temp_defpix[i] = 0;

        // top half
        if (deadpixadr[i] < (uint16_t)(NUMBER_OF_PIXEL / 2)) {

            if ( (deadpixmask[i] & 1 )  == 1) {
                number_neighbours[i]++;
                temp_defpix[i] = temp_defpix[i] + data_pixel[(deadpixadr[i] / PIXEL_PER_ROW) - 1][(deadpixadr[i] % PIXEL_PER_ROW)];
            }

            if ( (deadpixmask[i] & 2 )  == 2 ) {
                number_neighbours[i]++;
                temp_defpix[i] = temp_defpix[i] + data_pixel[(deadpixadr[i] / PIXEL_PER_ROW) - 1][(deadpixadr[i] % PIXEL_PER_ROW) + 1];
            }

            if ( (deadpixmask[i] & 4 )  == 4 ) {
                number_neighbours[i]++;
                temp_defpix[i] = temp_defpix[i] + data_pixel[(deadpixadr[i] / PIXEL_PER_ROW)][(deadpixadr[i] % PIXEL_PER_ROW) + 1];
            }

            if ( (deadpixmask[i] & 8 )  == 8 ) {
                number_neighbours[i]++;
                temp_defpix[i] = temp_defpix[i] + data_pixel[(deadpixadr[i] / PIXEL_PER_ROW) + 1][(deadpixadr[i] % PIXEL_PER_ROW) + 1];
            }

            if ( (deadpixmask[i] & 16 )  == 16 ) {
                number_neighbours[i]++;
                temp_defpix[i] = temp_defpix[i] + data_pixel[(deadpixadr[i] / PIXEL_PER_ROW) + 1][(deadpixadr[i] % PIXEL_PER_ROW)];
            }

            if ( (deadpixmask[i] & 32 )  == 32 ) {
                number_neighbours[i]++;
                temp_defpix[i] = temp_defpix[i] + data_pixel[(deadpixadr[i] / PIXEL_PER_ROW) + 1][(deadpixadr[i] % PIXEL_PER_ROW) - 1];
            }

            if ( (deadpixmask[i] & 64 )  == 64 ) {
                number_neighbours[i]++;
                temp_defpix[i] = temp_defpix[i] + data_pixel[(deadpixadr[i] / PIXEL_PER_ROW)][(deadpixadr[i] % PIXEL_PER_ROW) - 1];
            }

            if ( (deadpixmask[i] & 128 )  == 128 ) {
                number_neighbours[i]++;
                temp_defpix[i] = temp_defpix[i] + data_pixel[(deadpixadr[i] / PIXEL_PER_ROW) - 1][(deadpixadr[i] % PIXEL_PER_ROW) - 1];
            }

        }else {// bottom half

            if ( (deadpixmask[i] & 1 )  == 1 ) {
                number_neighbours[i]++;
                temp_defpix[i] = temp_defpix[i] + data_pixel[(deadpixadr[i] / PIXEL_PER_ROW) + 1][(deadpixadr[i] % PIXEL_PER_ROW)];
            }

            if ( (deadpixmask[i] & 2 )  == 2 ) {
                number_neighbours[i]++;
                temp_defpix[i] = temp_defpix[i] + data_pixel[(deadpixadr[i] / PIXEL_PER_ROW) + 1][(deadpixadr[i] % PIXEL_PER_ROW) + 1];
            }

            if ( (deadpixmask[i] & 4 )  == 4 ) {
                number_neighbours[i]++;
                temp_defpix[i] = temp_defpix[i] + data_pixel[(deadpixadr[i] / PIXEL_PER_ROW)][(deadpixadr[i] % PIXEL_PER_ROW) + 1];
            }

            if ( (deadpixmask[i] & 8 )  == 8 ) {
                number_neighbours[i]++;
                temp_defpix[i] = temp_defpix[i] + data_pixel[(deadpixadr[i] / PIXEL_PER_ROW) - 1][(deadpixadr[i] % PIXEL_PER_ROW) + 1];
            }

            if ( (deadpixmask[i] & 16 )  == 16 ) {
                number_neighbours[i]++;
                temp_defpix[i] = temp_defpix[i] + data_pixel[(deadpixadr[i] / PIXEL_PER_ROW) - 1][(deadpixadr[i] % PIXEL_PER_ROW)];
            }

            if ( (deadpixmask[i] & 32 )  == 32 ) {
                number_neighbours[i]++;
                temp_defpix[i] = temp_defpix[i] + data_pixel[(deadpixadr[i] / PIXEL_PER_ROW) - 1][(deadpixadr[i] % PIXEL_PER_ROW) - 1];
            }

            if ( (deadpixmask[i] & 64 )  == 64 ) {
                number_neighbours[i]++;
                temp_defpix[i] = temp_defpix[i] + data_pixel[(deadpixadr[i] / PIXEL_PER_ROW)][(deadpixadr[i] % PIXEL_PER_ROW) - 1];
            }

            if ( (deadpixmask[i] & 128 )  == 128 ) {
                number_neighbours[i]++;
                temp_defpix[i] = temp_defpix[i] + data_pixel[(deadpixadr[i] / PIXEL_PER_ROW) + 1][(deadpixadr[i] % PIXEL_PER_ROW) - 1];
            }
        }

        temp_defpix[i] = temp_defpix[i] / number_neighbours[i];
        data_pixel[deadpixadr[i] / PIXEL_PER_ROW][deadpixadr[i] % PIXEL_PER_ROW] = temp_defpix[i];

    }
}


/********************************************************************
   Function:      calcPixC
   Description:   calculates the pixel constants with the unscaled
                  values from EEPROM
 *******************************************************************/
void calcPixC() {

    /* uses the formula from datasheet:

                        PixC_uns[m][n]*(PixCmax-PixCmin)               epsilon   GlobalGain
        PixC[m][n] = ( -------------------------------- + PixCmin ) * ------- * ----------
                                    65535                               100        1000
    */

    double pixcij;
    pixc2 = pixc2_0; // set pointer to start address of the allocated heap

    for (int m = 0; m < DevConst.PixelPerColumn; m++) {
        for (int n = 0; n < DevConst.PixelPerRow; n++) {

        pixcij = (double)pixcmax;
        pixcij -= (double)pixcmin;
        pixcij /= (double)65535.0;
        pixcij *= (double) * pixc2;
        pixcij += (double)pixcmin;
        pixcij /= (double)100.0;
        pixcij *= (double)epsilon;
        pixcij /= (double)10000.0;
        pixcij *= (double)globalgain;
        pixcij += 0.5;

        *pixc2 = (uint32_t)pixcij;
        pixc2++;

        }
    }

    lastepsilon = epsilon;

}


/********************************************************************
   Function:        calculate_pixel_temp()
   Description:     compensate thermal, electrical offset and vdd and multiply sensitivity coeff
                    look for the correct temp in lookup table
 *******************************************************************/
void calculate_pixel_temp() {

    int64_t vij_pixc_and_pcscaleval;
    int64_t pixcij;
    int64_t vdd_calc_steps;
    uint16_t table_row, table_col;
    int32_t vx, vy, ydist, dta;
    int32_t pixel;
    pixc2 = pixc2_0; // set pointer to start address of the allocated heap


    /******************************************************************************************************************
        step 0: find column of lookup table
    ******************************************************************************************************************/
    for (int i = 0; i < NROFTAELEMENTS; i++) {
        if (Ta > XTATemps[i]) {
        table_col = i;
        }
    }
    dta = Ta - XTATemps[table_col];
    ydist = (int32_t)ADEQUIDISTANCE;


    flag_min_max_initaled = false;
    for (int m = 0; m < DevConst.PixelPerColumn; m++) {
        for (int n = 0; n < DevConst.PixelPerRow; n++) {

            /******************************************************************************************************************
            step 1: use a variable with bigger data format for the compensation steps
            ******************************************************************************************************************/
            prob_lock = true;
            pixel = (int32_t) data_pixel[m][n];
            prob_lock = false;
            /******************************************************************************************************************
            step 2: compensate thermal drifts (see datasheet, chapter: Thermal Offset)
            ******************************************************************************************************************/
            pixel -= (int32_t)(((int32_t)thgrad[m][n] * (int32_t)ptat_av_uint16) / (int32_t)gradscale_div);
            pixel -= (int32_t)thoffset[m][n];

            /******************************************************************************************************************
            step 3: compensate electrical offset (see datasheet, chapter: Electrical Offset)
            ******************************************************************************************************************/
            if (m < DevConst.PixelPerColumn / 2) { // top half
                pixel -= eloffset[m % DevConst.RowPerBlock][n];
            }
            else { // bottom half
                pixel -= eloffset[m % DevConst.RowPerBlock + DevConst.RowPerBlock][n];
            }

            /******************************************************************************************************************
            step 4: compensate vdd (see datasheet, chapter: Vdd Compensation)
            ******************************************************************************************************************/
            // first select VddCompGrad and VddCompOff for pixel m,n:
            if (m < DevConst.PixelPerColumn / 2) {      // top half
                vddcompgrad_n = vddcompgrad[m % DevConst.RowPerBlock][n];
                vddcompoff_n = vddcompoff[m % DevConst.RowPerBlock][n];
            }
            else {       // bottom half
                vddcompgrad_n = vddcompgrad[m % DevConst.RowPerBlock + DevConst.RowPerBlock][n];
                vddcompoff_n = vddcompoff[m % DevConst.RowPerBlock + DevConst.RowPerBlock][n];
            }
            // now do the vdd calculation
            vdd_calc_steps = vddcompgrad_n * ptat_av_uint16;
            vdd_calc_steps = vdd_calc_steps / vddscgrad_div;
            vdd_calc_steps = vdd_calc_steps + vddcompoff_n;
            vdd_calc_steps = vdd_calc_steps * ( vdd_av_uint16 - vddth1 - ((vddth2 - vddth1) / (ptatth2 - ptatth1)) * (ptat_av_uint16  - ptatth1));
            vdd_calc_steps = vdd_calc_steps / vddscoff_div;
            pixel -= vdd_calc_steps;

            /******************************************************************************************************************
            step 5: multiply sensitivity coeff for each pixel (see datasheet, chapter: Object Temperature)
            ******************************************************************************************************************/
            vij_pixc_and_pcscaleval = pixel * (int64_t)PCSCALEVAL;
            pixel =  (int32_t)(vij_pixc_and_pcscaleval / *pixc2);
            pixc2++;
            /******************************************************************************************************************
            step 6: find correct temp for this sensor in lookup table and do a bilinear interpolation (see datasheet, chapter:  Look-up table)
            ******************************************************************************************************************/
            table_row = pixel + TABLEOFFSET;
            table_row = table_row >> ADEXPBITS;
            // bilinear interpolation
            vx = ((((int32_t)TempTable[table_row][table_col + 1] - (int32_t)TempTable[table_row][table_col]) * (int32_t)dta) / (int32_t)TAEQUIDISTANCE) + (int32_t)TempTable[table_row][table_col];
            vy = ((((int32_t)TempTable[table_row + 1][table_col + 1] - (int32_t)TempTable[table_row + 1][table_col]) * (int32_t)dta) / (int32_t)TAEQUIDISTANCE) + (int32_t)TempTable[table_row + 1][table_col];
            pixel = (uint32_t)((vy - vx) * ((int32_t)(pixel + TABLEOFFSET) - (int32_t)YADValues[table_row]) / ydist + (int32_t)vx);

            /******************************************************************************************************************
            step 7: add GlobalOffset (stored as int8_t)
            ******************************************************************************************************************/
            pixel += globaloff;

            /******************************************************************************************************************
        step 8: overwrite the uncompensate pixel with the new calculated compensated value
            ******************************************************************************************************************/
            data_pixel[m][n] = (uint16_t)pixel;

            /******************************************************************************************************************
        step 9: find min and max value
            ******************************************************************************************************************/
            
            if (flag_min_max_initaled == false) {
                T_max = data_pixel[m][n];
                T_min = data_pixel[m][n];
                T_avg = data_pixel[m][n];
                flag_min_max_initaled = true;
            }
            if (n<30 && m<30 && n > 2 && m > 2) {
                if(data_pixel[m][n] < T_min) {
                T_min = data_pixel[m][n];
                    x_min = n;
                    y_min = m;
                }

                if(data_pixel[m][n] > T_max) {
                T_max = data_pixel[m][n];
                    x_max = n;
                    y_max = m;
                }
                T_avg = T_avg + data_pixel[m][n];
            }
        }
    }
    /******************************************************************************************************************
        step 8: overwrite the uncompensate pixel with the new calculated compensated value
    ******************************************************************************************************************/
    T_avg = T_avg / 729;
    pixel_masking();
}


/********************************************************************
   Function:      calc_timert(uint8_t clk, uint8_t mbit)
   Description:   calculate the duration of the timer which reads the sensor blocks
 *******************************************************************/
uint32_t calc_timert(uint8_t clk, uint8_t mbit) {

  float a;
  uint16_t calculated_timer_duration;

  float Fclk_float = 12000000.0 / 63.0 * (float)clk + 1000000.0;    // calc clk in Hz
  a = 32.0 * ((float)std::pow(2, (uint8_t)(mbit & 0b00001111)) + 4.0) / Fclk_float;

  calculated_timer_duration = (uint16_t)(0.98 * a * 1000000); // c in s | timer_duration in µs
  return calculated_timer_duration;
}



/********************************************************************
   Function:        void readblockinterrupt()
   Description:     read one sensor block and change configuration register to next block
                    (also read electrical offset when read_eloffset_next_pic is set)
 *******************************************************************/
void readblockinterrupt() {
    uint8_t bottomblock;
    ReadingRoutineEnable = 0;
    TimerLib.clearTimer();
    // clock::delay(15ms);
    // wait for end of conversion bit (~27ms)
    // check EOC bit
    read_sensor_register( STATUS_REGISTER, (uint8_t*)&statusreg, 1);
    // DEBUG_PRINT("statusreg: %d\n", statusreg);
    while (bitRead(statusreg, 0) == 0) {
        read_sensor_register( STATUS_REGISTER, (uint8_t*)&statusreg, 1);
        clock::delay(10ms);
    }
    // get data of top half:
    read_sensor_register( TOP_HALF, (uint8_t*)&RAMoutput[read_block_num], BLOCK_LENGTH);
    bottomblock = (uint8_t)((uint8_t)(NUMBER_OF_BLOCKS + 1) * 2 - read_block_num - 1);
    read_sensor_register( BOTTOM_HALF, (uint8_t*)&RAMoutput[bottomblock], BLOCK_LENGTH);

    read_block_num++;
    // DEBUG_PRINT("block %d sampled\n", read_block_num);
    if (read_block_num < NUMBER_OF_BLOCKS) {
        // to start sensor set configuration register to 0x09
        // |    RFU    |   Block   | Start | VDD_MEAS | BLIND | WAKEUP |
        // |  0  |  0  |  x  |  x  |   1   |    0     |   0   |    1   |
        write_sensor_byte(CONFIGURATION_REGISTER, (uint8_t)(0x09 + (0x10 * read_block_num) + (0x04 * switch_ptat_vdd)));
    }
    else {
        //*******************************************************************
        // all blocks for the current image are sampled, now check if its time
        // to get new electrical offsets and/or for switching PTAT and VDD
        //*******************************************************************
        // DEBUG_PRINTLN("all blocks for the current image are sampled");
        if (read_eloffset_next_pic) {
        read_eloffset_next_pic = 0;
        // |    RFU    |   Block   | Start | VDD_MEAS | BLIND | WAKEUP |
        // |  0  |  0  |  0  |  0  |   1   |    0     |   1   |    1   |
        write_sensor_byte(CONFIGURATION_REGISTER, (uint8_t)(0x0B + (0x04 * switch_ptat_vdd)));
        new_offsets = 1;
        }
        else {
        if (picnum > 1)
            state = 1; // state = 1 means that all required blocks are sampled
        picnum++; // increase the picture counter
        // check if the next sample routine should include electrical offsets
        if ((uint8_t)(picnum % READ_ELOFFSET_EVERYX) == 0)
            read_eloffset_next_pic = 1;
        if (DevConst.PTATVDDSwitch)
            switch_ptat_vdd ^= 1;
        read_block_num = 0;
        // |    RFU    |   Block   | Start | VDD_MEAS | BLIND | WAKEUP |
        // |  0  |  0  |  0  |  0  |   1   |    0     |   0   |    1   |
        write_sensor_byte(CONFIGURATION_REGISTER, (uint8_t)(0x09 + (0x04 * switch_ptat_vdd)));
        }
    }
    TimerLib.setInterval_us(ISR, timert);
    ReadingRoutineEnable = 1;
}


void read_eeprom() {
    int m = 0;
    int n = 0;
    uint8_t b[4];
    id = read_EEPROM_byte(E_ID4) << 24 | read_EEPROM_byte(E_ID3) << 16 | read_EEPROM_byte(E_ID2) << 8 | read_EEPROM_byte(E_ID1);
    mbit_calib = read_EEPROM_byte(E_MBIT_CALIB);
    bias_calib = read_EEPROM_byte(E_BIAS_CALIB);
    clk_calib = read_EEPROM_byte(E_CLK_CALIB);
    bpa_calib = read_EEPROM_byte(E_BPA_CALIB);
    pu_calib = read_EEPROM_byte(E_PU_CALIB);
    mbit_user = read_EEPROM_byte(E_MBIT_USER);
    bias_user = read_EEPROM_byte(E_BIAS_USER);
    clk_user = read_EEPROM_byte(E_CLK_USER);
    bpa_user = read_EEPROM_byte(E_BPA_USER);
    pu_user = read_EEPROM_byte(E_PU_USER);
    vddth1 = read_EEPROM_byte(E_VDDTH1_2) << 8 | read_EEPROM_byte(E_VDDTH1_1);
    vddth2 = read_EEPROM_byte(E_VDDTH2_2) << 8 | read_EEPROM_byte(E_VDDTH2_1);
    vddscgrad = read_EEPROM_byte(E_VDDSCGRAD);
    vddscoff = read_EEPROM_byte(E_VDDSCOFF);
    ptatth1 = read_EEPROM_byte(E_PTATTH1_2) << 8 | read_EEPROM_byte(E_PTATTH1_1);
    ptatth2 = read_EEPROM_byte(E_PTATTH2_2) << 8 | read_EEPROM_byte(E_PTATTH2_1);
    nrofdefpix = read_EEPROM_byte(E_NROFDEFPIX);
    gradscale = read_EEPROM_byte(E_GRADSCALE);
    tablenumber = read_EEPROM_byte(E_TABLENUMBER2) << 8 | read_EEPROM_byte(E_TABLENUMBER1);
    arraytype = read_EEPROM_byte(E_ARRAYTYPE);
    b[0] = read_EEPROM_byte(E_PTATGR_1);
    b[1] = read_EEPROM_byte(E_PTATGR_2);
    b[2] = read_EEPROM_byte(E_PTATGR_3);
    b[3] = read_EEPROM_byte(E_PTATGR_4);
    ptatgr_float = *(float*)b;
    b[0] = read_EEPROM_byte(E_PTATOFF_1);
    b[1] = read_EEPROM_byte(E_PTATOFF_2);
    b[2] = read_EEPROM_byte(E_PTATOFF_3);
    b[3] = read_EEPROM_byte(E_PTATOFF_4);
    ptatoff_float = *(float*)b;
    b[0] = read_EEPROM_byte(E_PIXCMIN_1);
    b[1] = read_EEPROM_byte(E_PIXCMIN_2);
    b[2] = read_EEPROM_byte(E_PIXCMIN_3);
    b[3] = read_EEPROM_byte(E_PIXCMIN_4);
    pixcmin = *(float*)b;
    b[0] = read_EEPROM_byte(E_PIXCMAX_1);
    b[1] = read_EEPROM_byte(E_PIXCMAX_2);
    b[2] = read_EEPROM_byte(E_PIXCMAX_3);
    b[3] = read_EEPROM_byte(E_PIXCMAX_4);
    pixcmax = *(float*)b;
    epsilon = read_EEPROM_byte(E_EPSILON);
    globaloff = read_EEPROM_byte(E_GLOBALOFF);
    globalgain = read_EEPROM_byte(E_GLOBALGAIN_2) << 8 | read_EEPROM_byte(E_GLOBALGAIN_1);


    // for (int m = 0; m < DevConst.PixelPerColumn; m++) {
    //   for (int n = 0; n < DevConst.PixelPerRow; n++) {

    // --- DeadPixAdr ---
    for (int i = 0; i < nrofdefpix; i++) {
        deadpixadr[i] = read_EEPROM_byte(E_DEADPIXADR + 2 * i + 1 ) << 8 | read_EEPROM_byte(E_DEADPIXADR + 2 * i);
        if (deadpixadr[i] > (uint16_t)(DevConst.NumberOfPixel / 2)) {  // adaptedAdr:
        deadpixadr[i] = (uint16_t)(DevConst.NumberOfPixel) + (uint16_t)(DevConst.NumberOfPixel / 2) - deadpixadr[i] + 2 * (uint16_t)(deadpixadr[i] % DevConst.PixelPerRow ) - DevConst.PixelPerRow;
        }
    }


    // --- DeadPixMask ---
    for (int i = 0; i < nrofdefpix; i++) {
        deadpixmask[i] = read_EEPROM_byte(E_DEADPIXMASK + i);
    }


    // --- Thgrad_ij, ThOffset_ij and P_ij ---
    m = 0;
    n = 0;
    pixc2 = pixc2_0; // set pointer to start address of the allocated heap // reset pointer to initial address
    // top half
    for (int i = 0; i < (uint16_t)(DevConst.NumberOfPixel / 2); i++) {
        thgrad[m][n] = read_EEPROM_byte(E_THGRAD + 2 * i + 1) << 8 | read_EEPROM_byte(E_THGRAD + 2 * i);
        thoffset[m][n] = read_EEPROM_byte(E_THOFFSET + 2 * i + 1) << 8 | read_EEPROM_byte(E_THOFFSET + 2 * i);
        *(pixc2 + m * DevConst.PixelPerRow + n) = read_EEPROM_byte(E_PIJ + 2 * i + 1) << 8 | read_EEPROM_byte(E_PIJ + 2 * i);
        n++;
        if (n ==  DevConst.PixelPerRow) {
        n = 0;
        m++;  // !!!! forwards !!!!
        }
    }
    // bottom half
    m = (uint8_t)(DevConst.PixelPerColumn - 1);
    n = 0;
    for (int i = (uint16_t)(DevConst.NumberOfPixel / 2); i < (uint16_t)(DevConst.NumberOfPixel); i++) {
        thgrad[m][n] = read_EEPROM_byte(E_THGRAD + 2 * i + 1) << 8 | read_EEPROM_byte(E_THGRAD + 2 * i);
        thoffset[m][n] = read_EEPROM_byte(E_THOFFSET + 2 * i + 1) << 8 | read_EEPROM_byte(E_THOFFSET + 2 * i);
        *(pixc2 + m * DevConst.PixelPerRow + n) = read_EEPROM_byte(E_PIJ + 2 * i + 1) << 8 | read_EEPROM_byte(E_PIJ + 2 * i);
        n++;

        if (n ==  DevConst.PixelPerRow) {
        n = 0;
        m--;      // !!!! backwards !!!!
        }
    }

    //---VddCompGrad and VddCompOff---
    // top half
    m = 0;
    n = 0;
    // top half
    for (int i = 0; i < (uint16_t)(DevConst.PixelPerBlock); i++) {
        vddcompgrad[m][n] = read_EEPROM_byte(E_VDDCOMPGRAD + 2 * i + 1) << 8 | read_EEPROM_byte(E_VDDCOMPGRAD + 2 * i);
        vddcompoff[m][n] = read_EEPROM_byte(E_VDDCOMPOFF + 2 * i + 1) << 8 | read_EEPROM_byte(E_VDDCOMPOFF + 2 * i);
        n++;
        if (n ==  DevConst.PixelPerRow) {
        n = 0;
        m++;  // !!!! forwards !!!!
        }
    }
    // bottom half
    m = (uint8_t)(DevConst.RowPerBlock * 2 - 1);
    n = 0;
    for (int i = (uint16_t)(DevConst.PixelPerBlock); i < (uint16_t)(DevConst.PixelPerBlock * 2); i++) {
        vddcompgrad[m][n] = read_EEPROM_byte(E_VDDCOMPGRAD + 2 * i + 1) << 8 | read_EEPROM_byte(E_VDDCOMPGRAD + 2 * i);
        vddcompoff[m][n] = read_EEPROM_byte(E_VDDCOMPOFF + 2 * i + 1) << 8 | read_EEPROM_byte(E_VDDCOMPOFF + 2 * i);
        n++;
        if (n ==  DevConst.PixelPerRow) {
        n = 0;
        m--;      // !!!! backwards !!!!
        }
    }

}


/********************************************************************
   Function:        void sort_data()
   Description:     sort the raw data blocks in 2d array and calculate ambient temperature, ptat and vdd
 *******************************************************************/
void sort_data() {

  uint32_t sum = 0;
  uint16_t pos = 0;

  for (int m = 0; m < DevConst.RowPerBlock; m++) {
    for (int n = 0; n < DevConst.PixelPerRow; n++) {

      /*
         for example: a normal line of RAMoutput for HTPAd80x64 looks like:
         RAMoutput[0][] = [ PTAT(MSB), PTAT(LSB), DATA0[MSB], DATA0[LSB], DATA1[MSB], DATA1[LSB], ... , DATA640[MSB], DATA640LSB];
                                                      |
                                                      |-- DATA_Pos = 2 (first data uint8_t)
      */
      pos = (uint16_t)(2 * n + DevConst.DataPos + m * 2 * DevConst.PixelPerRow);



      /******************************************************************************************************************
        new PIXEL values
      ******************************************************************************************************************/
      for (int i = 0; i < DevConst.NumberOfBlocks; i++) {
        // top half
        data_pixel[m + i * DevConst.RowPerBlock][n] =
          (uint16_t)(RAMoutput[i][pos] << 8 | RAMoutput[i][pos + 1]);
        // bottom half
        data_pixel[DevConst.PixelPerColumn - 1 - m - i * DevConst.RowPerBlock][n] =
          (uint16_t)(RAMoutput[2 * DevConst.NumberOfBlocks + 2 - i - 1][pos] << 8 | RAMoutput[2 * DevConst.NumberOfBlocks + 2 - i - 1][pos + 1]);
      }


      /******************************************************************************************************************
        new electrical offset values (store them in electrical offset buffer and calculate the average for pixel compensation
      ******************************************************************************************************************/
      if (picnum % ELOFFSETS_BUFFER_SIZE == 1) {
        if ((!eloffset[m][n]) || (picnum < ELOFFSETS_FILTER_START_DELAY)) {
          // top half
          eloffset[m][n] = (uint16_t)(RAMoutput[DevConst.NumberOfBlocks][pos] << 8 | RAMoutput[DevConst.NumberOfBlocks][pos + 1]);
          // bottom half
          eloffset[2 * DevConst.RowPerBlock - 1 - m][n] = (uint16_t)(RAMoutput[DevConst.NumberOfBlocks + 1][pos] << 8 | RAMoutput[DevConst.NumberOfBlocks + 1][pos + 1]);
          use_eloffsets_buffer = 1;

        }
        else {
          // use a moving average filter
          // top half
          sum = (uint32_t)eloffset[m][n] * (uint32_t)(ELOFFSETS_BUFFER_SIZE - 1);
          sum += (uint32_t)(RAMoutput[DevConst.NumberOfBlocks][pos] << 8 | RAMoutput[DevConst.NumberOfBlocks][pos + 1]);
          eloffset[m][n] = (uint16_t)((float)sum / ELOFFSETS_BUFFER_SIZE + 0.5);
          // bottom half
          sum = (uint32_t)eloffset[2 * DevConst.RowPerBlock - 1 - m][n] * (uint32_t)(ELOFFSETS_BUFFER_SIZE - 1);
          sum += (uint32_t)(RAMoutput[DevConst.NumberOfBlocks + 1][pos] << 8 | RAMoutput[DevConst.NumberOfBlocks + 1][pos + 1]);
          eloffset[2 * DevConst.RowPerBlock - 1 - m][n] = (uint16_t)((float)sum / ELOFFSETS_BUFFER_SIZE + 0.5);
        }
      }

    }

  }



  /******************************************************************************************************************
    new PTAT values (store them in PTAT buffer and calculate the average for pixel compensation
  ******************************************************************************************************************/
  if (switch_ptat_vdd == 1) {
    sum = 0;
    // calculate ptat average (datasheet, chapter: 11.1 Ambient Temperature )
    for (int i = 0; i < DevConst.NumberOfBlocks; i++) {
      // block top half
      sum += (uint16_t)(RAMoutput[i][DevConst.PTATPos] << 8 | RAMoutput[i][DevConst.PTATPos + 1]);
      // block bottom half
      sum += (uint16_t)(RAMoutput[2 * DevConst.NumberOfBlocks - i + 1][DevConst.PTATPos] << 8 | RAMoutput[2 * DevConst.NumberOfBlocks - i + 1][DevConst.PTATPos + 1]);
    }
    ptat_av_uint16 = (uint16_t)((float)sum / (float)(2.0 * DevConst.NumberOfBlocks));
    Ta = (uint16_t)((uint16_t)ptat_av_uint16 * (float)ptatgr_float + (float)ptatoff_float);


    ptat_buffer[ptat_i] = ptat_av_uint16;
    ptat_i++;
    if (ptat_i == PTAT_BUFFER_SIZE) {
      if (use_ptat_buffer == 0) {
        //DEBUG_PRINT(" | PTAT buffer complete");
        use_ptat_buffer = 1;
      }
      ptat_i = 0;
    }

    if (use_ptat_buffer) {
      // now overwrite the old ptat average
      sum = 0;
      for (int i = 0; i < PTAT_BUFFER_SIZE; i++) {
        sum += ptat_buffer[i];
      }
      ptat_av_uint16 = (uint16_t)((float)sum / PTAT_BUFFER_SIZE);
    }


  }


  /******************************************************************************************************************
    new VDD values (store them in VDD buffer and calculate the average for pixel compensation
  ******************************************************************************************************************/
  if (switch_ptat_vdd == 0) {
    sum = 0;
    // calculate vdd average (datasheet, chapter: 11.4 Vdd Compensation )
    for (int i = 0; i < DevConst.NumberOfBlocks; i++) {
      // block top half
      sum += (uint16_t)(RAMoutput[i][DevConst.VDDPos] << 8 | RAMoutput[i][DevConst.VDDPos + 1]);
      // block bottom half
      sum += (uint16_t)(RAMoutput[2 * DevConst.NumberOfBlocks - i + 1][DevConst.VDDPos] << 8 | RAMoutput[2 * DevConst.NumberOfBlocks - i + 1][DevConst.VDDPos + 1]);
    }
    vdd_av_uint16 = (uint16_t)((float)sum / (float)(2.0 * DevConst.NumberOfBlocks));


    // write into vdd buffer
    vdd_buffer[vdd_i] = vdd_av_uint16;
    vdd_i++;
    if (vdd_i == VDD_BUFFER_SIZE) {
      if (use_vdd_buffer == 0) {
        //DEBUG_PRINT(" | VDD buffer complete");
        use_vdd_buffer = 1;
      }
      vdd_i = 0;
    }
    if (use_vdd_buffer) {
      sum = 0;
      for (int i = 0; i < VDD_BUFFER_SIZE; i++) {
        sum += vdd_buffer[i];
      }
      // now overwrite the old vdd average
      vdd_av_uint16 = (uint16_t)((float)sum / VDD_BUFFER_SIZE);
    }

  }

}


/********************************************************************
   Function:        void write_calibration_settings_to_sensor()
   Description:     write calibration data (from eeprom) to trim registers (sensor)
 *******************************************************************/
void write_calibration_settings_to_sensor() {

  write_sensor_byte(TRIM_REGISTER1, mbit_calib);
  clock::delay(5ms);
  write_sensor_byte(TRIM_REGISTER2, bias_calib);
  clock::delay(5ms);
  write_sensor_byte(TRIM_REGISTER3, bias_calib);
  clock::delay(5ms);
  write_sensor_byte(TRIM_REGISTER4, clk_calib);
  clock::delay(5ms);
  write_sensor_byte(TRIM_REGISTER5, bpa_calib);
  clock::delay(5ms);
  write_sensor_byte(TRIM_REGISTER6, bpa_calib);
  clock::delay(5ms);
  write_sensor_byte(TRIM_REGISTER7, pu_calib);
  clock::delay(5ms);
}







/********************************************************************
   Function:        void write_user_settings_to_sensor()

   Description:     write calibration data (from eeprom) to trim registers (sensor)

   Dependencies:
 *******************************************************************/
void write_user_settings_to_sensor() {
  write_sensor_byte(TRIM_REGISTER1, mbit_user);
  clock::delay(5ms);
  write_sensor_byte(TRIM_REGISTER2, bias_user);
  clock::delay(5ms);
  write_sensor_byte(TRIM_REGISTER3, bias_user);
  clock::delay(5ms);
  write_sensor_byte(TRIM_REGISTER4, clk_user);
  clock::delay(5ms);
  write_sensor_byte(TRIM_REGISTER5, bpa_user);
  clock::delay(5ms);
  write_sensor_byte(TRIM_REGISTER6, bpa_user);
  clock::delay(5ms);
  write_sensor_byte(TRIM_REGISTER7, pu_user);
  clock::delay(5ms);
}


/********************************************************************
 ********************************************************************
    - - - PART 4: SERIAL FUNCTIONS - - -
  checkSerial()
  print_eeprom_header()
  print_eeprom_hex()
  print_menu()
 ********************************************************************
 ********************************************************************/
/********************************************************************
   Function:        print_eeprom_header()
   Description:
 *******************************************************************/
void print_eeprom_header() {
  DEBUG_PRINT("data\t\tregister\ttype\t\tvalue\n");
  DEBUG_PRINTLN("------------------------------------------------------------");
  DEBUG_PRINT("PixCmin\t\t0x00-0x03\tfloat\t\t");
  DEBUG_PRINTLN(pixcmin, 0);
  DEBUG_PRINT("PixCmax\t\t0x04-0x07\tfloat\t\t");
  DEBUG_PRINTLN(pixcmax, 0);
  DEBUG_PRINT("gradScale\t0x08\t\tuint8_t\t");
  DEBUG_PRINTLN(gradscale);
  DEBUG_PRINT("TN\t\t0x0B-0x0C\tuint16_t\t");
  DEBUG_PRINTLN(tablenumber);
  DEBUG_PRINT("epsilon\t\t0x0D\t\tuint8_t\t");
  DEBUG_PRINTLN(epsilon);
  DEBUG_PRINT("MBIT(calib)\t0x1A\t\tuint8_t\t");
  DEBUG_PRINTLN(mbit_calib);
  DEBUG_PRINT("BIAS(calib)\t0x1B\t\tuint8_t\t");
  DEBUG_PRINTLN(bias_calib);
  DEBUG_PRINT("CLK(calib)\t0x1C\t\tuint8_t\t");
  DEBUG_PRINTLN(clk_calib);
  DEBUG_PRINT("BPA(calib)\t0x1D\t\tuint8_t\t");
  DEBUG_PRINTLN(bpa_calib);
  DEBUG_PRINT("PU(calib)\t0x1E\t\tuint8_t\t");
  DEBUG_PRINTLN(pu_calib);
  DEBUG_PRINT("Arraytype\t0x22\t\tuint8_t\t");
  DEBUG_PRINTLN(arraytype);
  DEBUG_PRINT("VDDTH1\t\t0x26-0x27\tuint16_t\t");
  DEBUG_PRINTLN(vddth1);
  DEBUG_PRINT("VDDTH2\t\t0x28-0x29\tuint16_t\t");
  DEBUG_PRINTLN(vddth2);
  DEBUG_PRINT("PTAT-gradient\t0x34-0x37\tfloat\t\t");
  DEBUG_PRINTLN(ptatgr_float, 4);
  DEBUG_PRINT("PTAT-offset\t0x38-0x3B\tfloat\t\t");
  DEBUG_PRINTLN(ptatoff_float, 4);
  DEBUG_PRINT("PTAT(Th1)\t0x3C-0x3D\tuint16_t\t");
  DEBUG_PRINTLN(ptatth1);
  DEBUG_PRINT("PTAT(Th2)\t0x3E-0x3F\tuint16_t\t");
  DEBUG_PRINTLN(ptatth2);
  DEBUG_PRINT("VddScGrad\t0x4E\t\tuint8_t\t");
  DEBUG_PRINTLN(vddscgrad);
  DEBUG_PRINT("VddScOff\t0x4F\t\tuint8_t\t");
  DEBUG_PRINTLN(vddscoff);
  DEBUG_PRINT("GlobalOff\t0x54\t\tint8_t\t");
  DEBUG_PRINTLN(globaloff);
  DEBUG_PRINT("GlobalGain\t0x55-0x56\tuint16_t\t");
  DEBUG_PRINTLN(globalgain);
  DEBUG_PRINT("SensorID\t0x74-0x77\tuint32_t\t");
  DEBUG_PRINTLN(id);
}

/********************************************************************
   Function:        print_eeprom_hex()
   Description:     print eeprom contint as hex values
 *******************************************************************/
void print_eeprom_hex() {

    DEBUG_PRINT("\n\n\n---PRINT EEPROM (std::hex)---\n");
    DEBUG_PRINT("\n\n\t\t\t0x00\t0x01\t0x02\t0x03\t0x04\t0x05\t0x06\t0x07\t0x08\t0x09\t0x0A\t0x0B\t0x0C\t0x0D\t0x0E\t0x0F\n");

    // line
    for (int i = 0; i < 75; i++) {
        DEBUG_PRINT("- ");
    }

    for (int i = 0; i < EEPROM_SIZE; i++) {
        if (i % 16 == 0) {
            DEBUG_PRINT("\n");

            if (i < E_DEADPIXADR) {
                DEBUG_PRINT("HEADER\t0x");
                DEBUG_PRINT(i, std::hex);
                DEBUG_PRINT("\t|\t");
            }
            else if (i < 0x00D0) {
                DEBUG_PRINT("DEADPIX\t0x");
                DEBUG_PRINT(i, std::hex);
                DEBUG_PRINT("\t|\t");
            }
            else if (i < E_VDDCOMPGRAD) {
                DEBUG_PRINT("FREE\t0x");
                DEBUG_PRINT(i, std::hex);
                DEBUG_PRINT("\t|\t");
            }
            else if (i < E_VDDCOMPOFF) {
                DEBUG_PRINT("VDDGRAD\t0x");
                DEBUG_PRINT(i, std::hex);
                DEBUG_PRINT("\t|\t");
            }
            else if (i < E_THGRAD) {
                DEBUG_PRINT("VDDOFF\t0x");
                DEBUG_PRINT(i, std::hex);
                DEBUG_PRINT("\t|\t");
            }
            else if (i < E_THOFFSET) {
                DEBUG_PRINT("THGRAD\t0x");
                DEBUG_PRINT(i, std::hex);
                DEBUG_PRINT("\t|\t");
            }
            else if (i < E_PIJ) {
                DEBUG_PRINT("THOFF\t0x");
                DEBUG_PRINT(i, std::hex);
                DEBUG_PRINT("\t|\t");
            }
            else if (i < (E_PIJ + 2*NUMBER_OF_PIXEL)) {
                DEBUG_PRINT("PIXC\t0x");
                DEBUG_PRINT(i, std::hex);
                DEBUG_PRINT("\t|\t");
            }
            else {
                DEBUG_PRINT("FREE\t0x");
                DEBUG_PRINT(i, std::hex);
                DEBUG_PRINT("\t|\t");
            }
        }
        else {
            DEBUG_PRINT("\t");
        }

        DEBUG_PRINT("0x");
        if (read_EEPROM_byte(i) < 0x10) {
            DEBUG_PRINT("0");
        }
        DEBUG_PRINT(read_EEPROM_byte(i), std::hex);

    }

    DEBUG_PRINT("\n\n\n\ndone (m... back to menu)\n\n\n");
}



/********************************************************************
   Function:      print_menu()
   Description:
 *******************************************************************/
void print_menu() {
    DEBUG_PRINTLN("\n\n\n***************************************************");
    DEBUG_PRINTLN("Application Shield                      /_/eimann");
    DEBUG_PRINTLN("for ESP32-DevkitC                      / /   Sensor");

    DEBUG_PRINTLN("\nYou can choose one of these options by sending the \ncharacter\n ");
    DEBUG_PRINTLN("read SENSOR values:");
    DEBUG_PRINTLN("  a... final array temperatures (in deci Kelvin)");
    DEBUG_PRINTLN("  b... show all raw values (in digits)");
    DEBUG_PRINTLN("  c... show all calculation steps");
    DEBUG_PRINTLN("read EEPROM values:");
    DEBUG_PRINTLN("  d... whole eeprom content (in hexadecimal)");
    DEBUG_PRINTLN("  e... Header values");
    DEBUG_PRINTLN("  f... VddCompGrad");
    DEBUG_PRINTLN("  g... VddCompOff");
    DEBUG_PRINTLN("  h... ThGrad");
    DEBUG_PRINTLN("  i... ThOff");
    DEBUG_PRINTLN("  j... PixC (scaled)");
    DEBUG_PRINTLN("write/change EEPROM values:");
    DEBUG_PRINTLN("  k... increase emissivity by 1");
    DEBUG_PRINTLN("  l... decrease emissivity by 1");
    DEBUG_PRINTLN("\t\t\t\t\tver2.2 (dp)");
    DEBUG_PRINTLN("***************************************************\n\n\n");
}


/********************************************************************
   Function:        print_final_array()
   Description:
 *******************************************************************/
void print_final_array(void) {
  DEBUG_PRINTLN("\n\n---pixel data ---");
  for (int m = 0; m < DevConst.PixelPerColumn; m++) {
    for (int n = 0; n < DevConst.PixelPerRow; n++) {
      DEBUG_PRINT(data_pixel[m][n]);
      DEBUG_PRINT("\t");
    }
    DEBUG_PRINTLN("");
  }
}

/********************************************************************
   Function:        print_RAM_array()
   Description:
 *******************************************************************/
void print_RAM_array(void) {
  DEBUG_PRINT("\n\n\n---pixel data ---\n");
  for (int m = 0; m < (2 * NUMBER_OF_BLOCKS + 2); m++) {
    for (int n = 0; n < BLOCK_LENGTH; n++) {
      DEBUG_PRINT(RAMoutput[m][n], std::hex);
      DEBUG_PRINT("\t");
    }
    DEBUG_PRINT("\n");
  }
  DEBUG_PRINT("\n\n\n");
}

/********************************************************************
   Function:        checkSerial()
   Description:
 *******************************************************************/
void checkSerial(uint8_t serial_input) {

  switch (serial_input) {
    case 0xFF:
      //nothing
      break;

    case 'a':
        if (send_data)
            DEBUG_PRINTLN("stop data stream in GUI before");
        else
            print_state = 1;
        break;

    case 'b':
        if (send_data)
            DEBUG_PRINTLN("stop data stream in GUI before");
        else
            print_state = 2;
        break;

    case 'c':
        if (send_data)
            DEBUG_PRINTLN("stop data stream in GUI before");
        else
            print_state = 3;
        break;


    case 'm':
        while (state);
        ReadingRoutineEnable = 0;
        print_menu();
        ReadingRoutineEnable = 1;
        break;

    case 'd':
        while (state);
        ReadingRoutineEnable = 0;
        print_eeprom_hex();
        ReadingRoutineEnable = 1;
        break;

    case 'e':
        while (state);
        ReadingRoutineEnable = 0;
        print_eeprom_header();
        ReadingRoutineEnable = 1;
        break;

    case 'f':
        while (state);
        ReadingRoutineEnable = 0;
        DEBUG_PRINT("\n\n\n---VddCompGrad---\n");
        for (int m = 0; m < (DevConst.RowPerBlock * 2); m++) {
            for (int n = 0; n < DevConst.PixelPerRow; n++) {
            DEBUG_PRINT(vddcompgrad[m][n]);
            DEBUG_PRINT("\t");
            }
            DEBUG_PRINT("\n");
        }
        DEBUG_PRINT("\n\n\n");
        ReadingRoutineEnable = 1;
        break;

    case 'g':
        while (state);
        ReadingRoutineEnable = 0;
        DEBUG_PRINT("\n\n\n---VddCompOff---\n");
        for (int m = 0; m < (DevConst.RowPerBlock * 2); m++) {
            for (int n = 0; n < DevConst.PixelPerRow; n++) {
            DEBUG_PRINT(vddcompoff[m][n]);
            DEBUG_PRINT("\t");
            }
            DEBUG_PRINT("\n");
        }
        DEBUG_PRINT("\n\n\n");
        ReadingRoutineEnable = 1;
        break;

    case 'h':
        while (state);
        ReadingRoutineEnable = 0;
        DEBUG_PRINT("\n\n\n---ThGrad---\n");
        for (int m = 0; m < DevConst.PixelPerColumn; m++) {
            for (int n = 0; n < DevConst.PixelPerRow; n++) {
            DEBUG_PRINT(thgrad[m][n]);
            DEBUG_PRINT("\t");
            }
            DEBUG_PRINT("\n");
        }
        DEBUG_PRINT("\n\n\n");
        ReadingRoutineEnable = 1;
        break;



    case 'i':
        while (state);
        ReadingRoutineEnable = 0;
        // print ThOffset in serial monitor
        DEBUG_PRINT("\n\n\n---ThOffset---\n");
        for (int m = 0; m < DevConst.PixelPerColumn; m++) {
            for (int n = 0; n < DevConst.PixelPerRow; n++) {
            DEBUG_PRINT(thoffset[m][n]);
            DEBUG_PRINT("\t");
            }
            DEBUG_PRINT("\n");
        }
        DEBUG_PRINT("\n\n\n");
        ReadingRoutineEnable = 1;
        break;

    case 'j':
        while (state);
        ReadingRoutineEnable = 0;
        // print PixC in serial monitor
        DEBUG_PRINT("\n\n\n---PixC---\n");
        pixc2 = pixc2_0; // set pointer to start address of the allocated heap
        for (int m = 0; m < DevConst.PixelPerColumn; m++) {
            for (int n = 0; n < DevConst.PixelPerRow; n++) {
            DEBUG_PRINT(*(pixc2 + m * DevConst.PixelPerRow + n));
            DEBUG_PRINT("\t");
            }
            DEBUG_PRINT("\n");
        }
        DEBUG_PRINT("\n\n\n");
        ReadingRoutineEnable = 1;
        break;

    case 'k':
        ReadingRoutineEnable = 0;
        TimerLib.clearTimer();
        DEBUG_PRINTLN("\n\n\n---Increase emissivity---");
        DEBUG_PRINT("old emissivity: \t");
        DEBUG_PRINTLN(epsilon);
        DEBUG_PRINT("new emissivity: \t");
        if (epsilon < 100) {
            epsilon++;
            Wire.setClock(HTPA32X32_EEPROM_MAX_I2C_BAUDRATE); // I2C clock frequency 400kHz (for eeprom communication)
            write_EEPROM_byte(E_EPSILON, epsilon);
            Wire.setClock(CLOCK_SENSOR);

            // calculate pixcij with new epsilon
            pixc2 = pixc2_0; // set pointer to start address of the allocated heap
            double d = (double)epsilon / (double)lastepsilon;
            for (int m = 0; m < DevConst.PixelPerColumn; m++) {
            for (int n = 0; n < DevConst.PixelPerRow; n++) {
                *(pixc2 + m * DevConst.PixelPerRow + n) = (uint32_t)((double) * (pixc2 + m * DevConst.PixelPerRow + n) * (double)d);
            }
            }
            lastepsilon = epsilon;
            DEBUG_PRINT(epsilon);
            DEBUG_PRINTLN(" (new emissivity is stored in the EEPROM now)");
        }
        else {
            DEBUG_PRINT(epsilon);
            DEBUG_PRINTLN(" (you cannot set the emissivity higher than 100%)");
        }
        clock::delay(1000ms);
        TimerLib.setInterval_us(ISR, timert);
        ReadingRoutineEnable = 1;
        break;

    case 'l':
        ReadingRoutineEnable = 0;
        TimerLib.clearTimer();
        DEBUG_PRINT("\n\n\n---Decrease emissivity---");
        DEBUG_PRINT("\nold emissivity: \t");
        DEBUG_PRINT(epsilon);
        DEBUG_PRINT("\nnew emissivity: \t");
        if (epsilon > 0) {
            epsilon--;
            Wire.setClock(HTPA32X32_EEPROM_MAX_I2C_BAUDRATE); // I2C clock frequency 400kHz (for eeprom communication)
            write_EEPROM_byte(E_EPSILON, epsilon);
            Wire.setClock(CLOCK_SENSOR);
            // calculate pixcij with new epsilon
            pixc2 = pixc2_0; // set pointer to start address of the allocated heap
            double d = (double)epsilon / (double)lastepsilon;
            for (int m = 0; m < DevConst.PixelPerColumn; m++) {
            for (int n = 0; n < DevConst.PixelPerRow; n++) {
                *(pixc2 + m * DevConst.PixelPerRow + n) = (uint32_t)((double) * (pixc2 + m * DevConst.PixelPerRow + n) * (double)d);
            }
            }
            lastepsilon = epsilon;
            DEBUG_PRINT(epsilon);
            DEBUG_PRINT(" (new emissivity is stored in the EEPROM now)");
        }
        else {
            DEBUG_PRINT(epsilon);
            DEBUG_PRINT(" (you cannot set the emissivity lower as 0%)");
        }
        clock::delay(1000ms);
        TimerLib.setInterval_us(ISR, timert);
        ReadingRoutineEnable = 1;
        break;


  }


}


/********************************************************************
   Function:        print_calc_steps()
   Description:     print every needed step for temperature calculation + pixel masking
 *******************************************************************/
void print_calc_steps2() {
    int64_t vij_pixc_and_pcscaleval;
    int64_t pixcij;
    int64_t vdd_calc_steps;
    uint16_t table_row, table_col;
    int32_t vx, vy, ydist, dta;
    int32_t pixel;
    pixc2 = pixc2_0;

    DEBUG_PRINTLN("\n\ncalculate the average of VDD and PTAT buffer");

    DEBUG_PRINT("PTATbuf[");
    DEBUG_PRINT(PTAT_BUFFER_SIZE);
    DEBUG_PRINT("] = { ");
    for (int i = 0; i < PTAT_BUFFER_SIZE; i++) {
        DEBUG_PRINT(ptat_buffer[i]);
        if (i < (PTAT_BUFFER_SIZE - 1))
        DEBUG_PRINT(" , ");
        else
        DEBUG_PRINT(" }");
    }
    DEBUG_PRINT("\nPTAT_average = ");
    DEBUG_PRINT(ptat_av_uint16);

    DEBUG_PRINT("\nVDDbuf[");
    DEBUG_PRINT(VDD_BUFFER_SIZE);
    DEBUG_PRINT("] = { ");
    for (int i = 0; i < VDD_BUFFER_SIZE; i++) {
        DEBUG_PRINT(vdd_buffer[i]);
        if (i < (VDD_BUFFER_SIZE - 1))
        DEBUG_PRINT(" , ");
        else
        DEBUG_PRINT(" }");
    }
    DEBUG_PRINT("\nVDD_average = ");
    DEBUG_PRINT(vdd_av_uint16);

    DEBUG_PRINTLN("\n\ncalculate ambient temperatur (Ta)");
    DEBUG_PRINT("Ta = ");
    DEBUG_PRINT(ptat_av_uint16);
    DEBUG_PRINT(" * ");
    DEBUG_PRINT(ptatgr_float, 5);
    DEBUG_PRINT(" + ");
    DEBUG_PRINT(ptatoff_float, 5);
    DEBUG_PRINT(" = ");
    DEBUG_PRINT(Ta);
    DEBUG_PRINT(" (Value is given in dK)");


    /******************************************************************************************************************
        step 0: find column of lookup table
    ******************************************************************************************************************/
    for (int i = 0; i < NROFTAELEMENTS; i++) {
        if (Ta > XTATemps[i]) {
        table_col = i;
        }
    }
    dta = Ta - XTATemps[table_col];
    ydist = (int32_t)ADEQUIDISTANCE;

    DEBUG_PRINTLN("\n\nprint all calculation steps for each pixel");
    DEBUG_PRINTLN("table columns:");
    DEBUG_PRINTLN("No\tpixel number");
    DEBUG_PRINTLN("i\trepresents the row of the pixel");
    DEBUG_PRINTLN("j\trepresents the column of the pixel");
    DEBUG_PRINTLN("Vij\tis row pixel voltages (digital); readout from the RAM");
    DEBUG_PRINTLN("I\tis the thermal offset compensated voltage");
    DEBUG_PRINTLN("II\tis the thermal and electrical offset compensated voltage");
    DEBUG_PRINTLN("III\tis the Vdd compensated voltage");
    DEBUG_PRINTLN("IV\tis the sensivity compensated IR voltage");
    DEBUG_PRINTLN("T[dK]\tis final pixel temperature in dK (deci Kelvin)");
    DEBUG_PRINTLN("T[°C]\tis final pixel temperature in °C");

    DEBUG_PRINTLN("\n\nNo\ti\tj\tVij\tI\tII\tIII\tIV\tT[dK]\tT[°C]");
    DEBUG_PRINTLN("-----------------------------------------------------------------------------");


    for (int m = 0; m < DevConst.PixelPerColumn; m++) {
        for (int n = 0; n < DevConst.PixelPerRow; n++) {

            DEBUG_PRINT(m * DevConst.PixelPerRow + n);
            DEBUG_PRINT("\t");
            DEBUG_PRINT(m);
            DEBUG_PRINT("\t");
            DEBUG_PRINT(n);
            /******************************************************************************************************************
            step 1: use a variable with bigger data format for the compensation steps
            ******************************************************************************************************************/
            pixel = (int32_t) data_pixel[m][n];
            DEBUG_PRINT("\t"); DEBUG_PRINT(pixel);
            /******************************************************************************************************************
            step 2: compensate thermal drifts (see datasheet, chapter: Thermal Offset)
            ******************************************************************************************************************/
            pixel -= (int32_t)(((int32_t)thgrad[m][n] * (int32_t)ptat_av_uint16) / (int32_t)gradscale_div);
            pixel -= (int32_t)thoffset[m][n];
            DEBUG_PRINT("\t"); DEBUG_PRINT(pixel);
            /******************************************************************************************************************
            step 3: compensate electrical offset (see datasheet, chapter: Electrical Offset)
            ******************************************************************************************************************/
            if (m < DevConst.PixelPerColumn / 2) { // top half
                pixel -= eloffset[m % DevConst.RowPerBlock][n];
            }
            else { // bottom half
                pixel -= eloffset[m % DevConst.RowPerBlock + DevConst.RowPerBlock][n];
            }
            DEBUG_PRINT("\t"); DEBUG_PRINT(pixel);
            /******************************************************************************************************************
            step 4: compensate vdd (see datasheet, chapter: Vdd Compensation)
            ******************************************************************************************************************/
            // first select VddCompGrad and VddCompOff for pixel m,n:
            if (m < DevConst.PixelPerColumn / 2) {      // top half
                vddcompgrad_n = vddcompgrad[m % DevConst.RowPerBlock][n];
                vddcompoff_n = vddcompoff[m % DevConst.RowPerBlock][n];
            }
            else {       // bottom half
                vddcompgrad_n = vddcompgrad[m % DevConst.RowPerBlock + DevConst.RowPerBlock][n];
                vddcompoff_n = vddcompoff[m % DevConst.RowPerBlock + DevConst.RowPerBlock][n];
            }
            // now do the vdd calculation
            vdd_calc_steps = vddcompgrad_n * ptat_av_uint16;
            vdd_calc_steps = vdd_calc_steps / vddscgrad_div;
            vdd_calc_steps = vdd_calc_steps + vddcompoff_n;
            vdd_calc_steps = vdd_calc_steps * ( vdd_av_uint16 - vddth1 - ((vddth2 - vddth1) / (ptatth2 - ptatth1)) * (ptat_av_uint16  - ptatth1));
            vdd_calc_steps = vdd_calc_steps / vddscoff_div;
            pixel -= vdd_calc_steps;
            DEBUG_PRINT("\t"); DEBUG_PRINT(pixel);
            /******************************************************************************************************************
            step 5: multiply sensitivity coeff for each pixel (see datasheet, chapter: Object Temperature)
            ******************************************************************************************************************/
            vij_pixc_and_pcscaleval = pixel * (int64_t)PCSCALEVAL;
            pixel =  (int32_t)(vij_pixc_and_pcscaleval / *pixc2);
            pixc2++;
            DEBUG_PRINT("\t"); DEBUG_PRINT(pixel);
            /******************************************************************************************************************
            step 6: find correct temp for this sensor in lookup table and do a bilinear interpolation (see datasheet, chapter:  Look-up table)
            ******************************************************************************************************************/
            table_row = pixel + TABLEOFFSET;
            table_row = table_row >> ADEXPBITS;
            // bilinear interpolation
            vx = ((((int32_t)TempTable[table_row][table_col + 1] - (int32_t)TempTable[table_row][table_col]) * (int32_t)dta) / (int32_t)TAEQUIDISTANCE) + (int32_t)TempTable[table_row][table_col];
            vy = ((((int32_t)TempTable[table_row + 1][table_col + 1] - (int32_t)TempTable[table_row + 1][table_col]) * (int32_t)dta) / (int32_t)TAEQUIDISTANCE) + (int32_t)TempTable[table_row + 1][table_col];
            pixel = (uint32_t)((vy - vx) * ((int32_t)(pixel + TABLEOFFSET) - (int32_t)YADValues[table_row]) / ydist + (int32_t)vx);

            /******************************************************************************************************************
            step 7: add GlobalOffset (stored as int8_t)
            ******************************************************************************************************************/
            pixel += globaloff;
            DEBUG_PRINT("\t"); DEBUG_PRINT(pixel);
            DEBUG_PRINT("\t"); DEBUG_PRINT((float)((pixel - 2732) / 10.0));
            DEBUG_PRINT("\n");
            /******************************************************************************************************************
            step 8: overwrite the uncompensate pixel with the new calculated compensated value
            ******************************************************************************************************************/
            data_pixel[m][n] = (uint16_t)pixel;

        }
    }

    /******************************************************************************************************************
        step 8: overwrite the uncompensate pixel with the new calculated compensated value
    ******************************************************************************************************************/
    pixel_masking();
}


void sensor_init(){
    pixc2_0 = (uint32_t *)malloc(NUMBER_OF_PIXEL * 4);
    if (pixc2_0 == NULL)
    {
        DEBUG_PRINTLN("heap_caps_malloc failed");
    }
    else
    {
        DEBUG_PRINTLN("heap_caps_malloc succeeded");
        pixc2 = pixc2_0; // set pointer to start address of the allocated heap
    }
    #ifndef USE_SDK
    Wire.setClock(1000000);
    //*******************************************************************
    // searching for sensor; if connected: read the whole EEPROM
    //*******************************************************************
    uint8_t error = 1;
    while (error != 0) {
        DEBUG_PRINT("Triying to connect to HTPAd at address: %d\n", SENSOR_ADDRESS);
        clock::delay(2000ms);
        Wire.begin();
        Wire.beginTransmission(SENSOR_ADDRESS);
        error = Wire.endTransmission();
        DEBUG_PRINT("HTPAd is not ready, retrying..., error code: %d\n", error);
        }
        prob_status = PROB_INITIALIZING;
        Wire.setClock(HTPA32X32_EEPROM_MAX_I2C_BAUDRATE); // I2C clock frequency 400kHz (for eeprom communication)
        read_eeprom();
        // I2C clock frequency (for sensor communication)
        Wire.setClock(CLOCK_SENSOR);
        #else
        ... TODO
        #endif
        //*******************************************************************
        // wake up and start the sensor
        //*******************************************************************
        // to wake up sensor set configuration register to 0x01
        // |    RFU    |   Block   | Start | VDD_MEAS | BLIND | WAKEUP |
        // |  0  |  0  |  0  |  0  |   0   |    0     |   0   |    1   |
        write_sensor_byte(CONFIGURATION_REGISTER, 0x01);
        // write the calibration settings into the trim registers
        write_calibration_settings_to_sensor();
        // to start sensor set configuration register to 0x09
        // |    RFU    |   Block   | Start | VDD_MEAS | BLIND | WAKEUP |
        // |  0  |  0  |  0  |  0  |   1   |    0     |   0   |    1   |
        write_sensor_byte(CONFIGURATION_REGISTER, 0x09);
        DEBUG_PRINTLN("HTPAd is ready");
        prob_status = PROB_PREPARING;
        //*******************************************************************
        // do bigger calculation here before you jump into the loop() function
        //*******************************************************************
        gradscale_div = std::pow(2, gradscale);
        vddscgrad_div = std::pow(2, vddscgrad);
        vddscoff_div = std::pow(2, vddscoff);
        calcPixC(); // calculate the pixel constants
        //*******************************************************************
        // timer initialization
        //*******************************************************************
        timert = calc_timert(clk_calib, mbit_calib);
        TimerLib.setInterval_us(ISR, timert);
        DEBUG_PRINT("calc_timert: %d\n", timert);
}

void sensor_power_on(){
  pinMode(MLX_VDD, OUTPUT);
  digitalWrite(MLX_VDD, LOW);
  prob_status = PROB_CONNECTING;
}


// 更新传感器画面
void sensor_loop(){
  // uint32_t t0 = millis();
  // TimerLib.timerLoop();
  
  NewDataAvailable = true;
  if (NewDataAvailable) {
    readblockinterrupt();
    NewDataAvailable = 0;
    clock::delay(1ms);
    // DEBUG_PRINT("read_block cost :%d\n", millis() - t0);
  }

  if (state) { // state is 1 when all raw sensor voltages are read for this picture
    while (pix_cp_lock == true) {clock::delay(2ms);}
    prob_lock = true;
    sort_data();
    state = 0;
    calculate_pixel_temp();
    prob_lock = false;
    // DEBUG_PRINT("max, min, avg: %d, %d, %d\n", T_max, T_min, T_avg);
  }
}
};

}