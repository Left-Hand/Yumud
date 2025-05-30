#include "LSM303.hpp"

using namespace ymd;
using namespace ymd::drivers;


// Defines ////////////////////////////////////////////////////////////////

// The Arduino two-wire interface uses a 7-bit number for the address,
// and sets the last bit correctly based on reads and writes
#define D_SA0_HIGH_ADDRESS                0b0011101
#define D_SA0_LOW_ADDRESS                 0b0011110
#define DLHC_DLM_DLH_MAG_ADDRESS          0b0011110
#define DLHC_DLM_DLH_ACC_SA0_HIGH_ADDRESS 0b0011001
#define DLM_DLH_ACC_SA0_LOW_ADDRESS       0b0011000

#define TEST_REG_ERROR -1

#define D_WHO_ID    0x49
#define DLM_WHO_ID  0x3C

#define TEST_REG(addr, test_bits) ({\
    uint8_t dummy;\
    const auto res = readReg(addr, dummy);\
    if(res.is_err()) return Err(res.unwrap_err());\
    (dummy & test_bits);\
})\

using Error = LSM303::Error;

template<typename T = void>
using IResult = Result<T, Error>;


// IResult<> LSM303::init(deviceType device, sa0State sa0)
// {
//     // perform auto-detection unless device type and SA0 state were both specified
//     if (device == device_auto || sa0 == sa0_auto)
//     {
//         // check for LSM303D if device is unidentified or was specified to be this type
//         if (device == device_auto || device == device_D)
//         {
//         // check SA0 high address unless SA0 was specified to be low
//         if (sa0 != sa0_low && TEST_REG(D_SA0_HIGH_ADDRESS, WHO_AM_I) == D_WHO_ID)
//         {
//             // device responds to address 0011101 with D ID; it's a D with SA0 high
//             device = device_D;
//             sa0 = sa0_high;
//         }
//         // check SA0 low address unless SA0 was specified to be high
//         else if (sa0 != sa0_high && TEST_REG(D_SA0_LOW_ADDRESS, WHO_AM_I) == D_WHO_ID)
//         {
//             // device responds to address 0011110 with D ID; it's a D with SA0 low
//             device = device_D;
//             sa0 = sa0_low;
//         }
//         }
        
//         // check for LSM303DLHC, DLM, DLH if device is still unidentified or was specified to be one of these types
//         if (device == device_auto || device == device_DLHC || device == device_DLM || device == device_DLH)
//         {
//         // check SA0 high address unless SA0 was specified to be low
//         if (sa0 != sa0_low && TEST_REG(DLHC_DLM_DLH_ACC_SA0_HIGH_ADDRESS, CTRL_REG1_A) != TEST_REG_ERROR)
//         {
//             // device responds to address 0011001; it's a DLHC, DLM with SA0 high, or DLH with SA0 high
//             sa0 = sa0_high;
//             if (device == device_auto)
//             { 
//             // use magnetometer WHO_AM_I register to determine device type
//             //
//             // DLHC seems to respond to WHO_AM_I request the same way as DLM, even though this
//             // register isn't documented in its datasheet. Since the DLHC accelerometer address is the
//             // same as the DLM with SA0 high, but Pololu DLM boards pull SA0 low by default, we'll
//             // guess that a device whose accelerometer responds to the SA0 high address and whose
//             // magnetometer gives the DLM ID is actually a DLHC.
//             device = (TEST_REG(DLHC_DLM_DLH_MAG_ADDRESS, WHO_AM_I_M) == DLM_WHO_ID) ? device_DLHC : device_DLH;
//             }
//         }
//         // check SA0 low address unless SA0 was specified to be high
//         else if (sa0 != sa0_high && TEST_REG(DLM_DLH_ACC_SA0_LOW_ADDRESS, CTRL_REG1_A) != TEST_REG_ERROR)
//         {
//             // device responds to address 0011000; it's a DLM with SA0 low or DLH with SA0 low
//             sa0 = sa0_low;
//             if (device == device_auto)
//             {
//             // use magnetometer WHO_AM_I register to determine device type
//             device = (TEST_REG(DLHC_DLM_DLH_MAG_ADDRESS, WHO_AM_I_M) == DLM_WHO_ID) ? device_DLM : device_DLH;
//             }
//         }
//         }
        
//         // make sure device and SA0 were successfully detected; otherwise, indicate failure
//         if (device == device_auto || sa0 == sa0_auto)
//         {
//             // return false;
//             return Err(Error::)
//         }
//     }
    
//     _device = device;
    
//     // set device addresses and translated register addresses
//     switch (device)
//     {
//         case device_D:
//         acc_address = mag_address = (sa0 == sa0_high) ? D_SA0_HIGH_ADDRESS : D_SA0_LOW_ADDRESS;
//         translated_regs[-OUT_X_L_M] = D_OUT_X_L_M;
//         translated_regs[-OUT_X_H_M] = D_OUT_X_H_M;
//         translated_regs[-OUT_Y_L_M] = D_OUT_Y_L_M;
//         translated_regs[-OUT_Y_H_M] = D_OUT_Y_H_M;
//         translated_regs[-OUT_Z_L_M] = D_OUT_Z_L_M;
//         translated_regs[-OUT_Z_H_M] = D_OUT_Z_H_M;
//         break;

//         case device_DLHC:
//         acc_address = DLHC_DLM_DLH_ACC_SA0_HIGH_ADDRESS; // DLHC doesn't have configurable SA0 but uses same acc address as DLM/DLH with SA0 high
//         mag_address = DLHC_DLM_DLH_MAG_ADDRESS;
//         translated_regs[-OUT_X_H_M] = DLHC_OUT_X_H_M;
//         translated_regs[-OUT_X_L_M] = DLHC_OUT_X_L_M;
//         translated_regs[-OUT_Y_H_M] = DLHC_OUT_Y_H_M;
//         translated_regs[-OUT_Y_L_M] = DLHC_OUT_Y_L_M;
//         translated_regs[-OUT_Z_H_M] = DLHC_OUT_Z_H_M;
//         translated_regs[-OUT_Z_L_M] = DLHC_OUT_Z_L_M;
//         break;

//         case device_DLM:
//         acc_address = (sa0 == sa0_high) ? DLHC_DLM_DLH_ACC_SA0_HIGH_ADDRESS : DLM_DLH_ACC_SA0_LOW_ADDRESS;
//         mag_address = DLHC_DLM_DLH_MAG_ADDRESS;
//         translated_regs[-OUT_X_H_M] = DLM_OUT_X_H_M;
//         translated_regs[-OUT_X_L_M] = DLM_OUT_X_L_M;
//         translated_regs[-OUT_Y_H_M] = DLM_OUT_Y_H_M;
//         translated_regs[-OUT_Y_L_M] = DLM_OUT_Y_L_M;
//         translated_regs[-OUT_Z_H_M] = DLM_OUT_Z_H_M;
//         translated_regs[-OUT_Z_L_M] = DLM_OUT_Z_L_M;
//         break;

//         case device_DLH:
//         acc_address = (sa0 == sa0_high) ? DLHC_DLM_DLH_ACC_SA0_HIGH_ADDRESS : DLM_DLH_ACC_SA0_LOW_ADDRESS;
//         mag_address = DLHC_DLM_DLH_MAG_ADDRESS;
//         translated_regs[-OUT_X_H_M] = DLH_OUT_X_H_M;
//         translated_regs[-OUT_X_L_M] = DLH_OUT_X_L_M;
//         translated_regs[-OUT_Y_H_M] = DLH_OUT_Y_H_M;
//         translated_regs[-OUT_Y_L_M] = DLH_OUT_Y_L_M;
//         translated_regs[-OUT_Z_H_M] = DLH_OUT_Z_H_M;
//         translated_regs[-OUT_Z_L_M] = DLH_OUT_Z_L_M;
//         break;

//         default:
//             __builtin_unreachable();
//     }
    
//     return true;
// }

/*
Enables the LSM303's accelerometer and magnetometer. Also:
- Sets sensor full scales (gain) to default power-on values, which are
    +/- 2 g for accelerometer and +/- 1.3 gauss for magnetometer
    (+/- 4 gauss on LSM303D).
- Selects 50 Hz ODR (output data rate) for accelerometer and 7.5 Hz
    ODR for magnetometer (6.25 Hz on LSM303D). (These are the ODR
    settings for which the electrical characteristics are specified in
    the datasheets.)
- Enables high resolution modes (if available).
Note that this function will also reset other settings controlled by
the registers it writes to.
*/
IResult<> LSM303::enable_default()
{

    if (_device == device_D)
    {
    // Accelerometer

    // 0x00 = 0b00000000
    // AFS = 0 (+/- 2 g full scale)
    if(const auto res = write_reg(CTRL2, 0x00);
        res.is_err()) return res;

    // 0x57 = 0b01010111
    // AODR = 0101 (50 Hz ODR); AZEN = AYEN = AXEN = 1 (all axes enabled)
    if(const auto res = write_reg(CTRL1, 0x57);
        res.is_err()) return res;

    // Magnetometer

    // 0x64 = 0b01100100
    // M_RES = 11 (high resolution mode); M_ODR = 001 (6.25 Hz ODR)
    if(const auto res = write_reg(CTRL5, 0x64);
        res.is_err()) return res;

    // 0x20 = 0b00100000
    // MFS = 01 (+/- 4 gauss full scale)
    if(const auto res = write_reg(CTRL6, 0x20);
        res.is_err()) return res;

    // 0x00 = 0b00000000
    // MLP = 0 (low power mode off); MD = 00 (continuous-conversion mode)
    if(const auto res = write_reg(CTRL7, 0x00);
        res.is_err()) return res;
    }
    else
    {
    // Accelerometer
    
    if (_device == device_DLHC)
    {
        // 0x08 = 0b00001000
        // FS = 00 (+/- 2 g full scale); HR = 1 (high resolution enable)
        if(const auto res = write_acc_reg(CTRL_REG4_A, 0x08);
            res.is_err()) return res;

        // 0x47 = 0b01000111
        // ODR = 0100 (50 Hz ODR); LPen = 0 (normal mode); Zen = Yen = Xen = 1 (all axes enabled)
        if(const auto res = write_acc_reg(CTRL_REG1_A, 0x47);
            res.is_err()) return res;
    }
    else // DLM, DLH
    {
        // 0x00 = 0b00000000
        // FS = 00 (+/- 2 g full scale)
        if(const auto res = write_acc_reg(CTRL_REG4_A, 0x00);
            res.is_err()) return res;

        // 0x27 = 0b00100111
        // PM = 001 (normal mode); DR = 00 (50 Hz ODR); Zen = Yen = Xen = 1 (all axes enabled)
        if(const auto res = write_acc_reg(CTRL_REG1_A, 0x27);
            res.is_err()) return res;
    }

    // Magnetometer

    // 0x0C = 0b00001100
    // DO = 011 (7.5 Hz ODR)
    if(const auto res = write_mag_reg(CRA_REG_M, 0x0C);
        res.is_err()) return res;

    // 0x20 = 0b00100000
    // GN = 001 (+/- 1.3 gauss full scale)
    if(const auto res = write_mag_reg(CRB_REG_M, 0x20);
        res.is_err()) return res;

    // 0x00 = 0b00000000
    // MD = 00 (continuous-conversion mode)
    if(const auto res = write_mag_reg(MR_REG_M, 0x00);
        res.is_err()) return res;
  }
  return Ok();
}

// Writes an accelerometer register
IResult<> LSM303::write_acc_reg(uint8_t reg, uint8_t value)
{
    TODO();
    return Ok();
}


// Writes a magnetometer register
IResult<> LSM303::write_mag_reg(uint8_t reg, uint8_t value)
{
    TODO();
    return Ok();
}

IResult<> LSM303::write_reg(uint8_t reg, uint8_t value)
{
    TODO();
    return Ok();
}

// Note that this function will not work for reading TEMP_OUT_H_M and TEMP_OUT_L_M on the DLHC.
// To read those two registers, use readMagReg() instead.
IResult<> LSM303::read_reg(int reg, uint8_t & data)
{
    TODO();
    return Ok();
}

// Reads the 3 accelerometer channels and stores them in Vector3 a
IResult<> LSM303::read_acc()
{
    TODO();
    return Ok();
}

IResult<> LSM303::read_mag()
{
    TODO();
    return Ok();
}