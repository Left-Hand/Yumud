//这个驱动尚未完成

//这段代码基于 https://github.com/TomSievers/vl53l1x-rs/blob/master/src/lib.rs 改编
//基本与rust代码保持一致

//! Port of the STM IMG009 Ultra Lite Driver for the VL53L1X.
//!
//! # Features
//!
//! This crate has one feature called `i2c-iter`.
//! This feature changes the communication implementation of the sensor.
//! With this feature enabled the I2C instance is expected to implement
//! the [`WriteIter`](embedded_hal::blocking::i2c::WriteIter) and [`WriteIterRead`](embedded_hal::blocking::i2c::WriteIterRead) traits instead of [`Write`](embedded_hal::blocking::i2c::Write) and [`WriteRead`]((embedded_hal::blocking::i2c::Write)) traits.
//! The iterator implementation has the advantage that a call to [`write_bytes`](crate::VL53L1X::write_bytes())
//! is not limited to a slice of 4 bytes.
//!
//! # Example
//!
//! ```
//! use vl53l1x_uld::{self, VL53L1X, IOVoltage, RangeStatus, SENSOR_ID};
//! # use embedded_hal_mock::i2c::{Mock, Transaction};
//! # use cfg_if::cfg_if;
//! #
//! # fn create_i2c() -> Mock {
//! #    auto expectations = vec![
//! #        Transaction::write_read(0x29, vec![0x01, 0x0F], vec![0xEA, 0xCC]),
//! #        Transaction::write(0x29, vec![0x00, 0x2D, 0x00]),
//! #        Transaction::write(0x29, vec![0x00, 0x2F, 0x01]),
//! #        Transaction::write(0x29, vec![0x00, 0x2E, 0x01]),
//! #    ];
//! #
//! #    cfg_if! {
//! #        if #[cfg(feature = "i2c-iter")] {
//! #            expectations.push(Transaction::write(0x29, vec![0x00, 0x30].iter().chain(VL53L1X::<Mock>::DEFAULT_CONFIG.iter()).cloned().collect()));
//! #        } else {
//! #            for (byte, address) in VL53L1X::<Mock>::DEFAULT_CONFIG.iter().zip(0x30u16..0x88) {
//! #                const auto adrs = address.to_be_bytes();
//! #                expectations.push(Transaction::write(0x29, vec![adrs[0], adrs[1], *byte]));
//! #            }
//! #        }
//! #    }
//! #
//! #    expectations.append(&mut vec![
//! #        Transaction::write(0x29, vec![0x00, 0x87, 0x40]),
//! #        Transaction::write_read(0x29, vec![0x00, 0x30], vec![0x00]),
//! #        Transaction::write_read(0x29, vec![0x00, 0x31], vec![0x01]),
//! #        Transaction::write(0x29, vec![0x00, 0x86, 0x01]),
//! #        Transaction::write(0x29, vec![0x00, 0x87, 0x00]),
//! #        Transaction::write(0x29, vec![0x00, 0x08, 0x09]),
//! #        Transaction::write(0x29, vec![0x00, 0x0B, 0x00]),
//! #        Transaction::write(0x29, vec![0x00, 0x87, 0x40]),
//! #        Transaction::write_read(0x29, vec![0x00, 0x30], vec![0x00]),
//! #        Transaction::write_read(0x29, vec![0x00, 0x31], vec![0x01]),
//! #        Transaction::write_read(0x29, vec![0x00, 0x89], vec![0x09]),
//! #        Transaction::write_read(0x29, vec![0x00, 0x96], vec![0x00, 0x0F]),
//! #    ]);
//! #
//! #    Mock::new(&expectations)
//! # }
//! #
//! // Create hardware specific I2C instance.
//! const auto i2c = create_i2c();
//! // Create sensor with default address.
//! auto vl = VL53L1X::new(i2c, vl53l1x_uld::DEFAULT_ADDRESS);
//!
//! const ERR : &str = "Failed to communicate";
//!
//! // Check if the sensor id is correct.
//! if (vl.get_sensor_id().expect(ERR) == SENSOR_ID)
//! {
//!     // Initialize the sensor before any usage.
//!     // Set the voltage of the IO pins to be 2.8 volts
//!     vl.init(IOVoltage::Volt2_8).expect(ERR);
//!
//!     // Start a ranging operation, needed to retrieve a distance
//!     vl.start_ranging().expect(ERR);
//!     
//!     // Wait until distance data is ready to be read.
//!     while !vl.is_data_ready().expect(ERR) {}
//!
//!     // Check if ditance measurement is valid.
//!     if (vl.get_range_status().expect(ERR) == RangeStatus::Valid)
//!     {
//!         // Retrieve measured distance.
//!         const auto distance = vl.get_distance().expect(ERR);
//!     }
//! }
//!
//!
//! ```

#pragma once

#include "hal/bus/i2c/i2cdrv.hpp"
#include "core/utils/Result.hpp"
#include "details/reg_map.hpp"

namespace ymd::drivers{

struct SwVersion{
    uint8_t major;
    uint8_t minor;
    uint8_t build;
    uint32_t revision;
};

class Window{
    enum Kind:uint8_t{
        Below,
        Above,
        In,
        Out,
    };

    constexpr Window(Kind kind):kind_(kind){}

    static constexpr Window from_u8(const uint8_t raw){
        switch(raw){
            case 0 : return {Kind::Below};
            case 1 : return {Kind::Above};
            case 2 : return {Kind::In};
            case 3 : return {Kind::Out};
            default : return {Kind::Below};
        }
    }
private:
    Kind kind_;
};


struct Threshold{
    uint16_t low;
    uint16_t high;
    Window window;
};


class VL53L1X_Error{
public:
    enum Kind:uint8_t{
        CommunicationError,
        InvalidTimingBudget,
        InvalidDistanceMode,
        InvalidSigmaThreshold,
    };

    VL53L1X_Error(Kind kind):kind_(kind){}

    bool operator == (Kind kind) const {
        return kind_ == kind;
    }
private:
    Kind kind_;
};


enum class Polarity:uint8_t{
    ActiveHigh = 0,
    ActiveLow = 1,
};


enum class DistanceMode:uint8_t{
    Short = 1,
    Long = 2,
};

class RangeStatus{
    enum Kind:uint8_t{
        /// Valid measurement.
        Valid = 0,
        /// Sigma is above threshold (possibly valid measurement).
        SigmaFailure = 1,
        /// Signal is above threshold (possibly valid measurement).
        SignalFailure = 2,
        /// Target is below minimum detection threshold.
        MinRangeClipped = 3,
        /// Phase is out of bounds.
        OutOfBounds = 4,
        /// HW or VCSEL failure.
        HardwareFailure = 5,
        /// Valid range, but wraparound check has not been done.
        WrapCheckFail = 6,
        /// Wrapped target, non matching phases.
        Wraparound = 7,
        /// Internal algorithm underflow or overflow.
        ProcessingFailure = 8,
        /// Crosstalk between signals.
        CrosstalkSignal = 9,
        /// First interrupt when starting ranging in back to back mode. Ignore measurement.
        Synchronisation = 10,
        /// Valid measurement but result is from multiple merging pulses.
        MergedPulse = 11,
        /// Used by RQL as different to phase fail.
        LackOfSignal = 12,
        /// Target is below minimum detection threshold.
        MinRangeFail = 13,
        /// Measurement is invalid.
        InvalidRange = 14,
        /// No new data.
        None = 255,
    };

    RangeStatus(Kind kind) : kind_(kind) {}

    static constexpr RangeStatus from_u8(const uint8_t raw){
        switch(v){
            case 3 : return {Kind::HardwareFailure};
            case 4 : return {Kind::SignalFailure};
            case 5 : return {Kind::OutOfBounds};
            case 6 : return {Kind::SigmaFailure};
            case 7 : return {Kind::Wraparound};
            case 8 : return {Kind::MinRangeClipped};
            case 9 : return {Kind::Valid};
            case 12: return  {Kind::CrosstalkSignal};
            case 13: return  {Kind::MinRangeFail};
            case 18: return  {Kind::Synchronisation};
            case 19: return  {Kind::WrapCheckFail};
            case 22: return  {Kind::MergedPulse};
            case 23: return  {Kind::LackOfSignal};
            default : return {Kind::None};
        }
    }

    bool operator ==(const RangeStatus & other) const{
        return kind_ == other.kind_;
    }
private:
    Kind kind_;
};

enum class IoVoltage{
    /// The IO voltage is 1.8v
    Volt1_8,
    /// The IO voltage is 2.8v
    Volt2_8,
};



struct ROI{
    uint16_t width;
    uint16_t height;
};

struct ROICenter{
    uint8_t spad;

    ROICenter(const uint8_t x, const uint8_t y):
        spd(y > 7 ? (128 + (x << 3) + (15 - y)) : ((15-x) << 3) + y){}
};


struct MeasureResult{
    RangeStatue status;
    uint16_t distance_mm;
    uint16_t ambient;
    uint16_t sig_per_spad;
    uint16_t spad_count;
};

/// Default I2C address for VL53L1X.
static constexpr uint8_t DEFAULT_I2C_ADDR= 0x29;

/// The ID of the sensor as returned by the device when querying [`VL53L1X::get_sensor_id`].
static constexpr uint16_t SENSOR_ID = 0xEACC;

/// Default configuration used during initialization.
static constexpr uint8_t DEFAULT_CONFIG[] = {
    0x01, /* 0x30 : set bit 4 to 0 for active high interrupt and 1 for active low (bits 3:0 must be 0x1), use SetInterruptPolarity() */
    0x02, /* 0x31 : bit 1 = interrupt depending on the polarity, use CheckForDataReady() */
    0x00, /* 0x32 : not user-modifiable */
    0x02, /* 0x33 : not user-modifiable */
    0x08, /* 0x34 : not user-modifiable */
    0x00, /* 0x35 : not user-modifiable */
    0x08, /* 0x36 : not user-modifiable */
    0x10, /* 0x37 : not user-modifiable */
    0x01, /* 0x38 : not user-modifiable */
    0x01, /* 0x39 : not user-modifiable */
    0x00, /* 0x3a : not user-modifiable */
    0x00, /* 0x3b : not user-modifiable */
    0x00, /* 0x3c : not user-modifiable */
    0x00, /* 0x3d : not user-modifiable */
    0xff, /* 0x3e : not user-modifiable */
    0x00, /* 0x3f : not user-modifiable */
    0x0F, /* 0x40 : not user-modifiable */
    0x00, /* 0x41 : not user-modifiable */
    0x00, /* 0x42 : not user-modifiable */
    0x00, /* 0x43 : not user-modifiable */
    0x00, /* 0x44 : not user-modifiable */
    0x00, /* 0x45 : not user-modifiable */
    0x20, /* 0x46 : interrupt configuration 0->level low detection, 1-> level high, 2-> Out of window, 3->In window, 0x20-> New sample ready , TBC */
    0x0b, /* 0x47 : not user-modifiable */
    0x00, /* 0x48 : not user-modifiable */
    0x00, /* 0x49 : not user-modifiable */
    0x02, /* 0x4a : not user-modifiable */
    0x0a, /* 0x4b : not user-modifiable */
    0x21, /* 0x4c : not user-modifiable */
    0x00, /* 0x4d : not user-modifiable */
    0x00, /* 0x4e : not user-modifiable */
    0x05, /* 0x4f : not user-modifiable */
    0x00, /* 0x50 : not user-modifiable */
    0x00, /* 0x51 : not user-modifiable */
    0x00, /* 0x52 : not user-modifiable */
    0x00, /* 0x53 : not user-modifiable */
    0xc8, /* 0x54 : not user-modifiable */
    0x00, /* 0x55 : not user-modifiable */
    0x00, /* 0x56 : not user-modifiable */
    0x38, /* 0x57 : not user-modifiable */
    0xff, /* 0x58 : not user-modifiable */
    0x01, /* 0x59 : not user-modifiable */
    0x00, /* 0x5a : not user-modifiable */
    0x08, /* 0x5b : not user-modifiable */
    0x00, /* 0x5c : not user-modifiable */
    0x00, /* 0x5d : not user-modifiable */
    0x01, /* 0x5e : not user-modifiable */
    0xcc, /* 0x5f : not user-modifiable */
    0x0f, /* 0x60 : not user-modifiable */
    0x01, /* 0x61 : not user-modifiable */
    0xf1, /* 0x62 : not user-modifiable */
    0x0d, /* 0x63 : not user-modifiable */
    0x01, /* 0x64 : Sigma threshold MSB (mm in 14.2 format for MSB+LSB), use SetSigmaThreshold(), default value 90 mm  */
    0x68, /* 0x65 : Sigma threshold LSB */
    0x00, /* 0x66 : Min count Rate MSB (MCPS in 9.7 format for MSB+LSB), use SetSignalThreshold() */
    0x80, /* 0x67 : Min count Rate LSB */
    0x08, /* 0x68 : not user-modifiable */
    0xb8, /* 0x69 : not user-modifiable */
    0x00, /* 0x6a : not user-modifiable */
    0x00, /* 0x6b : not user-modifiable */
    0x00, /* 0x6c : Intermeasurement period MSB, 32 bits register, use SetIntermeasurementInMs() */
    0x00, /* 0x6d : Intermeasurement period */
    0x0f, /* 0x6e : Intermeasurement period */
    0x89, /* 0x6f : Intermeasurement period LSB */
    0x00, /* 0x70 : not user-modifiable */
    0x00, /* 0x71 : not user-modifiable */
    0x00, /* 0x72 : distance threshold high MSB (in mm, MSB+LSB), use SetD:tanceThreshold() */
    0x00, /* 0x73 : distance threshold high LSB */
    0x00, /* 0x74 : distance threshold low MSB ( in mm, MSB+LSB), use SetD:tanceThreshold() */
    0x00, /* 0x75 : distance threshold low LSB */
    0x00, /* 0x76 : not user-modifiable */
    0x01, /* 0x77 : not user-modifiable */
    0x0f, /* 0x78 : not user-modifiable */
    0x0d, /* 0x79 : not user-modifiable */
    0x0e, /* 0x7a : not user-modifiable */
    0x0e, /* 0x7b : not user-modifiable */
    0x00, /* 0x7c : not user-modifiable */
    0x00, /* 0x7d : not user-modifiable */
    0x02, /* 0x7e : not user-modifiable */
    0xc7, /* 0x7f : ROI center, use SetROI() */
    0xff, /* 0x80 : XY ROI (X=Width, Y=Height), use SetROI() */
    0x9B, /* 0x81 : not user-modifiable */
    0x00, /* 0x82 : not user-modifiable */
    0x00, /* 0x83 : not user-modifiable */
    0x00, /* 0x84 : not user-modifiable */
    0x01, /* 0x85 : not user-modifiable */
    0x00, /* 0x86 : clear interrupt, use ClearInterrupt() */
    0x00, /* 0x87 : start ranging, use StartRanging() or StopRanging(), If you want an automatic start after VL53L1X_init() call, put 0x40 in location 0x87 */
};

// pub use vl53l1_reg::Index as Register;

class VL53L1X{
    using Error = VL53L1X_Error;
    using Self = VL53L1X;

    /// Get the driver version.
    SwVersion sw_version() const {
        return{
            .major = 3,
            .minor = 5,
            .build = 1,
            .revision = 0,
        };
    }


    Result<void, Error> write_bytes(uint8_t address, const std::array<uint8_t, 2> bytes){
        // this->i2c
        //     .write_registers(this->address, address.into(), bytes);

        return Ok();
    }

    Result<void, Error> read_bytes(uint8_t address, const std::span<uint8_t, 2> bytes){
        // this->i2c
        //     .write_registers(this->address, address.into(), bytes);

        return Ok();
    }


    /// Set the I2C address of the current device in case multiple devices with the same address exists on the same bus.
    ///
    /// # Arguments
    ///
    /// * `new_address` - The new address to set for the current device.
    auto set_address(uint8_t new_address) -> Result<void, Error> {
        this->write_bytes(VL53L1_I2C_SLAVE__DEVICE_ADDRESS, {new_address, 0});

        this->address = new_address;

        return Ok();
    }

    /// Load the 88 byte default values for sensor initialisation.
    ///
    /// # Arguments
    ///
    /// * `io_config` - The io voltage that will be configured for the device.
    auto init(IoVoltage io_config) -> Result<void, Error> {
        this->write_bytes(VL53L1_PAD_I2C_HV__CONFIG, {0, 0});

        const uint8_t io = (io_config == IoVoltage::Volt2_8) ? 1 : 0;

        this->write_bytes(VL53L1_GPIO_HV_PAD__CTRL, {io, 0});
        this->write_bytes(VL53L1_PAD_I2C_HV__EXTSUP_CONFIG, {io, 0});

        TODO();
        // cfg_if! {
        //     if #[cfg(feature = "i2c-iter")] {
        //         this->write_bytes(VL53L1_GPIO_HV_MUX__CTRL, &Self::DEFAULT_CONFIG);
        //     } else {
        //         for (byte, address) in Self::DEFAULT_CONFIG.iter().zip(0x30u16..0x88) {
        //             this->write_bytes(address.to_be_bytes(), &[*byte]);
        //         }
        //     }
        // }

        this->start_ranging();

        while (!this->is_data_ready()) {}

        this->clear_interrupt();
        this->stop_ranging();

        this->write_bytes(VL53L1_VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND, {0x09, 0});
        this->write_bytes(VL53L1_VHV_CONFIG__INIT, {0, 0});

        return Ok();
    }

    /// Clear the interrupt flag on the device.
    /// Should be called after reading ranging data from the device to start the next measurement.
    auto clear_interrupt() -> Result<void, Error> {
        return this->write_bytes(VL53L1_SYSTEM__INTERRUPT_CLEAR, {0x01, 0});
    }

    /// Set polarity of the interrupt.
    ///
    /// # Arguments
    ///
    /// * `polarity` - The polarity to set.
    auto set_interrupt_polarity(Polarity polarity) -> Result<void, Error> {
        auto gpio_mux_hv = std::array<uint8_t, 2>{0, 0};

        this->read_bytes(VL53L1_GPIO_HV_MUX__CTRL, gpio_mux_hv);

        gpio_mux_hv[0] &= 0xEF;

        gpio_mux_hv[0] |= (uint8_t(polarity)) << 4;

        return this->write_bytes(VL53L1_GPIO_HV_MUX__CTRL, gpio_mux_hv);
    }

    /// Get the currently set interrupt polarity.
    auto get_interrupt_polarity() -> Result<Polarity, Error> {
        auto gpio_mux_hv = std::array<uint8_t, 2>{0, 0};

        this->read_bytes(VL53L1_GPIO_HV_MUX__CTRL, gpio_mux_hv);

        return Ok(Polarity(gpio_mux_hv[0] & 0x10));
    }

    /// Start a distance ranging operation.
    /// This operation is continuous, the interrupt flag should be cleared between each interrupt to start a new distance measurement.
    auto start_ranging() -> Result<void, Error> {
        return this->write_bytes(VL53L1_SYSTEM__MODE_START, {0x40, 0});
    }

    /// Stop an ongoing ranging operation.
    auto stop_ranging() -> Result<void, Error> {
        return this->write_bytes(VL53L1_SYSTEM__MODE_START, {0x00, 0});
    }

    /// Check if new ranging data is available by polling the device.
    auto is_data_ready() -> Result<bool, Error> {
        const auto polarity = uint8_t(this->get_interrupt_polarity().unwrap());

        auto state = std::array<uint8_t, 2>{0, 0};

        this->read_bytes(VL53L1_GPIO__TIO_HV_STATUS, state);

        if ((state[0] & 0x01) == polarity){
            return Ok(true);
        }

        return Ok(false);
    }

    /// Set the timing budget for a measurement operation in milliseconds.
    ///
    /// # Arguments
    ///
    /// * `milliseconds` - One of the following values = 15, 20, 33, 50, 100(default), 200, 500.
    auto set_timing_budget_ms(uint16_t milliseconds) -> Result<void, Error> {
        const auto mode = this->get_distance_mode();

        const auto (a, b) = match mode {
            DistanceMode::Short => match milliseconds {
                15 => (0x01Du16, 0x0027u16),
                20 => (0x051, 0x006E),
                33 => (0x00D6, 0x006E),
                50 => (0x1AE, 0x01E8),
                100 => (0x02E1, 0x0388),
                200 => (0x03E1, 0x0496),
                500 => (0x0591, 0x05C1),
                _ => return Err(Error::InvalidTimingBudget),
            },
            DistanceMode::Long => match milliseconds {
                20 => (0x001E, 0x0022),
                33 => (0x0060, 0x006E),
                50 => (0x00AD, 0x00C6),
                100 => (0x01CC, 0x01EA),
                200 => (0x02D9, 0x02F8),
                500 => (0x048F, 0x04A4),
                _ => return Err(Error::InvalidTimingBudget),
            },
        };

        this->write_bytes(
            VL53L1_RANGE_CONFIG__TIMEOUT_MACROP_A_HI,
            &a.to_be_bytes(),
        );
        this->write_bytes(
            VL53L1_RANGE_CONFIG__TIMEOUT_MACROP_B_HI,
            &b.to_be_bytes(),
        );

        return Ok();
    }

    /// Get the currently set timing budget of the device.
    auto get_timing_budget_ms() -> Result<uint16_t, Error> {
        auto a = std::array<uint8_t, 2>{0, 0};

        this->read_bytes(VL53L1_RANGE_CONFIG__TIMEOUT_MACROP_A_HI, a);

        switch((a[0] << 8) | a[1]){
            case (0x001D         ):        return Ok(15 );
            case (0x0051 | 0x001E):        return Ok(20 );
            case (0x00D6 | 0x0060):        return Ok(33 );
            case (0x01AE | 0x00AD):        return Ok(50 );
            case (0x02E1 | 0x01CC):        return Ok(100);
            case (0x03E1 | 0x02D9):        return Ok(200);
            case (0x0591 | 0x048F):        return Ok(500);
            default : return Err(Error::InvalidTimingBudget);
        }
    }

    /// Set the distance mode in which the device operates.
    ///
    /// # Arguments
    ///
    /// * `mode` - The distance mode to use.
    auto set_distance_mode(DistanceMode mode) -> Result<void, Error> {
        const auto tb = this->get_timing_budget_ms();

        TODO();
        const auto (timeout, vcsel_a, vcsel_b, phase, woi_sd0, phase_sd0) = match mode {
            DistanceMode::Short => (0x14u8, 0x07u8, 0x05u8, 0x38u8, 0x0705u16, 0x0606u16),
            DistanceMode::Long => (0x0A, 0x0F, 0x0D, 0xB8, 0x0F0D, 0x0E0E),
        };

        this->write_bytes(
            VL53L1_PHASECAL_CONFIG__TIMEOUT_MACROP,
            &timeout.to_be_bytes(),
        );
        this->write_bytes(
            VL53L1_RANGE_CONFIG__VCSEL_PERIOD_A,
            &vcsel_a.to_be_bytes(),
        );
        this->write_bytes(
            VL53L1_RANGE_CONFIG__VCSEL_PERIOD_B,
            &vcsel_b.to_be_bytes(),
        );
        this->write_bytes(
            VL53L1_RANGE_CONFIG__VALID_PHASE_HIGH,
            &phase.to_be_bytes(),
        );
        this->write_bytes(VL53L1_SD_CONFIG__WOI_SD0, &woi_sd0.to_be_bytes());
        this->write_bytes(
            VL53L1_SD_CONFIG__INITIAL_PHASE_SD0,
            &phase_sd0.to_be_bytes(),
        );

        this->set_timing_budget_ms(tb);

        return Ok();
    }

    /// Get the currently set distance mode of the device.
    Result<DistanceMode, Error> get_distance_mode() {
        auto out = std::array<uint8_t, 2>{0, 0};
        this->read_bytes(VL53L1_PHASECAL_CONFIG__TIMEOUT_MACROP, out);

        if (out[0] == 0x14) {
            return Ok(DistanceMode::Short);
        } else if (out[0] == 0x0A) {
            return Ok(DistanceMode::Long);
        } else {
            return Err(Error::InvalidDistanceMode);
        }
    }

    /// Set the inter measurement period in milliseconds.
    /// This value must be greater or equal to the timing budget. This condition is not checked by this driver.
    ///
    /// # Arguments
    ///
    /// * `milliseconds` - The number of milliseconds used for the inter measurement period.
    auto set_inter_measurement_period_ms(milliseconds: u16) -> Result<void, Error> {
        auto clock_pll = [0u8, 0];

        this->read_bytes(VL53L1_RESULT__OSC_CALIBRATE_VAL, &mut clock_pll);

        const auto clock_pll = u16::from_be_bytes(clock_pll) & 0x3FF;

        const auto val = ((clock_pll * milliseconds) as f32 * 1.075f32) as u32;

        this->write_bytes(
            VL53L1_SYSTEM__INTERMEASUREMENT_PERIOD,
            &val.to_be_bytes(),
        );

        return Ok();
    }

    /// Get the currently set inter measurement period in milliseconds.
    auto get_inter_measurement_period_ms() -> Result<u16, Error> {
        auto clock_pll = [0u8, 0];
        auto period = [0u8, 0, 0, 0];

        this->read_bytes(VL53L1_RESULT__OSC_CALIBRATE_VAL, &mut clock_pll);

        const auto clock_pll = u16::from_be_bytes(clock_pll) & 0x3FF;

        this->read_bytes(VL53L1_SYSTEM__INTERMEASUREMENT_PERIOD, &mut period);

        const auto period = u32::from_be_bytes(period);

        Ok((period / (clock_pll as f32 * 1.065f32) as u32) as u16)
    }

    /// Check if the device is booted.
    auto is_booted() -> Result<bool, Error> {
        auto status = std::array<uint8_t, 2>{0, 0};
        this->read_bytes(VL53L1_FIRMWARE__SYSTEM_STATUS, &mut status);

        Ok(status[0] == 1)
    }

    /// Get the sensor id of the sensor. This id must be equivalent to [`SENSOR_ID`].
    auto get_sensor_id() -> Result<u16, Error> {
        auto id = [0u8, 0];

        this->read_bytes(VL53L1_IDENTIFICATION__MODEL_ID, &mut id);

        Ok(u16::from_be_bytes(id))
    }

    /// Get the distance measured in millimeters.
    auto get_distance() -> Result<u16, Error> {
        auto distance = [0u8, 0];

        this->read_bytes(
            VL53L1_RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0,
            &mut distance,
        );

        Ok(u16::from_be_bytes(distance))
    }

    /// Get the returned signal per SPAD in kcps/SPAD where kcps stands for Kilo Count Per Second.
    auto get_signal_per_spad() -> Result<u16, Error> {
        auto signal = std::array<uint8_t, 2>{0, 0};
        auto spad_count = std::array<uint8_t, 2>{0, 0};

        this->read_bytes(
            VL53L1_RESULT__PEAK_SIGNAL_COUNT_RATE_CROSSTALK_CORRECTED_MCPS_SD0,
            &mut signal,
        );
        this->read_bytes(
            VL53L1_RESULT__DSS_ACTUAL_EFFECTIVE_SPADS_SD0,
            &mut spad_count,
        );

        const auto signal = u16::from_be_bytes(signal);
        const auto spad_count = u16::from_be_bytes(spad_count);

        Ok((200.0f32 * signal as f32 / spad_count as f32).min(u16::MAX as f32) as u16)
    }

    /// Get the ambient signal per SPAD in kcps/SPAD where kcps stands for Kilo Count Per Second.
    auto get_ambient_per_spad() -> Result<u16, Error> {
        auto spad_count = std::array<uint8_t, 2>{0, 0};
        auto ambient = std::array<uint8_t, 2>{0, 0};

        this->read_bytes(
            VL53L1_RESULT__DSS_ACTUAL_EFFECTIVE_SPADS_SD0,
            &mut spad_count,
        );
        this->read_bytes(VL53L1_RESULT__AMBIENT_COUNT_RATE_MCPS_SD0, &mut ambient);

        const auto spad_count = u16::from_be_bytes(spad_count);
        const auto ambient = u16::from_be_bytes(ambient);

        Ok((200.0f32 * ambient as f32 / spad_count as f32).min(u16::MAX as f32) as u16)
    }

    /// Get the returned signal in kcps (Kilo Count Per Second).
    auto get_signal_rate() -> Result<u16, Error> {
        auto signal = std::array<uint8_t, 2>{0, 0};

        this->read_bytes(
            VL53L1_RESULT__PEAK_SIGNAL_COUNT_RATE_CROSSTALK_CORRECTED_MCPS_SD0,
            &mut signal,
        );

        Ok(u16::from_be_bytes(signal).saturating_mul(8))
    }

    /// Get the count of currently enabled SPADs.
    auto get_spad_count() -> Result<u16, Error> {
        auto spad_count = std::array<uint8_t, 2>{0, 0};

        this->read_bytes(
            VL53L1_RESULT__DSS_ACTUAL_EFFECTIVE_SPADS_SD0,
            &mut spad_count,
        );

        Ok(u16::from_be_bytes(spad_count) >> 8)
    }

    /// Get the ambient signal in kcps (Kilo Count Per Second).
    auto get_ambient_rate() -> Result<u16, Error> {
        auto ambient = std::array<uint8_t, 2>{0, 0};

        this->read_bytes(VL53L1_RESULT__AMBIENT_COUNT_RATE_MCPS_SD0, &mut ambient);

        Ok(u16::from_be_bytes(ambient).saturating_mul(8))
    }

    /// Get the ranging status.
    auto get_range_status() -> Result<RangeStatus, Error> {
        auto status = [0];

        this->read_bytes(VL53L1_RESULT__RANGE_STATUS, &mut status);

        const auto status = u8::from_be_bytes(status) & 0x1F;

        Ok(status.into())
    }

    /// Get a measurement result object which is read in a single access.
    auto get_result() -> Result<MeasureResult, Error> {
        auto result = [0; 17];

        this->read_bytes(VL53L1_RESULT__RANGE_STATUS, &mut result);

        Ok(MeasureResult {
            status: (result[0] & 0x1F).into(),
            ambient: u16::from_be_bytes([result[7], result[8]]).saturating_mul(8),
            spad_count: result[3] as u16,
            sig_per_spad: u16::from_be_bytes([result[15], result[16]]).saturating_mul(8),
            distance_mm: u16::from_be_bytes([result[13], result[14]]),
        })
    }

    /// Set a offset in millimeters which is aplied to the distance.
    ///
    /// # Arguments
    ///
    /// * `offset` - The offset in millimeters.
    auto set_offset(offset: i16) -> Result<void, Error> {
        this->write_bytes(
            VL53L1_ALGO__PART_TO_PART_RANGE_OFFSET_MM,
            &(offset.saturating_mul(4)).to_be_bytes(),
        );

        this->write_bytes(VL53L1_MM_CONFIG__INNER_OFFSET_MM, &std::array<uint8_t, 2>{0, 0});
        this->write_bytes(VL53L1_MM_CONFIG__OUTER_OFFSET_MM, &std::array<uint8_t, 2>{0, 0});

        return Ok();
    }

    /// Get the current offset in millimeters.
    auto get_offset() -> Result<i16, Error> {
        auto offset = std::array<uint8_t, 2>{0, 0};

        this->read_bytes(VL53L1_ALGO__PART_TO_PART_RANGE_OFFSET_MM, &mut offset);

        auto offset = u16::from_be_bytes(offset) << 3;

        offset >>= 5;

        Ok(offset as i16)
    }

    /// Set the crosstalk correction value in cps (Count Per Second).
    ///
    /// # Arguments
    ///
    /// * `correction` - The number of photons reflected back from the cover glass in cps.
    auto set_cross_talk(correction: u16) -> Result<void, Error> {
        this->write_bytes(
            VL53L1_ALGO__CROSSTALK_COMPENSATION_X_PLANE_GRADIENT_KCPS,
            &std::array<uint8_t, 2>{0, 0},
        );
        this->write_bytes(
            VL53L1_ALGO__CROSSTALK_COMPENSATION_Y_PLANE_GRADIENT_KCPS,
            &std::array<uint8_t, 2>{0, 0},
        );

        const auto correction = (correction << 9) / 1000;

        this->write_bytes(
            VL53L1_ALGO__CROSSTALK_COMPENSATION_PLANE_OFFSET_KCPS,
            &correction.to_be_bytes(),
        );

        return Ok();
    }

    /// Get the crosstalk correction value in cps.
    auto get_cross_talk() -> Result<u16, Error> {
        auto correction = std::array<uint8_t, 2>{0, 0};

        this->read_bytes(
            VL53L1_ALGO__CROSSTALK_COMPENSATION_PLANE_OFFSET_KCPS,
            &mut correction,
        );

        const auto correction =
            ((u16::from_be_bytes(correction) as u32 * 1000) >> 9).min(u16::MAX as u32) as u16;

        Ok(correction)
    }

    /// Set a distance threshold.
    ///
    /// # Arguments
    ///
    /// * `threshold` - The threshold to apply.
    auto set_distance_threshold(threshold: Threshold) -> Result<void, Error> {
        auto config = [0];

        this->read_bytes(VL53L1_SYSTEM__INTERRUPT_CONFIG_GPIO, &mut config);

        const auto config = config[0] & 0x47 | (threshold.window as u8 & 0x07);

        this->write_bytes(VL53L1_SYSTEM__INTERRUPT_CONFIG_GPIO, {config, 0});
        this->write_bytes(VL53L1_SYSTEM__THRESH_HIGH, &threshold.high.to_be_bytes());
        this->write_bytes(VL53L1_SYSTEM__THRESH_LOW, &threshold.low.to_be_bytes());

        return Ok();
    }

    /// Get the currently set distance threshold.
    auto get_distance_threshold() -> Result<Threshold, Error> {
        auto config = [0];

        this->read_bytes(VL53L1_SYSTEM__INTERRUPT_CONFIG_GPIO, &mut config);

        const auto window: Window = (config[0] & 0x07).into();

        auto high = std::array<uint8_t, 2>{0, 0};
        auto low = std::array<uint8_t, 2>{0, 0};

        this->read_bytes(VL53L1_SYSTEM__THRESH_HIGH, &mut high);
        this->read_bytes(VL53L1_SYSTEM__THRESH_LOW, &mut low);

        Ok(Threshold {
            window,
            low: u16::from_be_bytes(low),
            high: u16::from_be_bytes(high),
        })
    }

    /// Set the region of interest of the sensor. The ROI is centered and only the size is settable.
    /// The smallest acceptable ROI size is 4.
    ///
    /// # Arguments
    ///
    /// * `roi` - The ROI to apply.
    auto set_roi(mut roi: ROI) -> Result<void, Error> {
        debug_assert!(roi.width >= 4);
        debug_assert!(roi.height >= 4);

        auto center = [0];

        this->read_bytes(VL53L1_ROI_CONFIG__MODE_ROI_CENTRE_SPAD, &mut center);

        if roi.width > 16 {
            roi.width = 16;
        }

        if roi.height > 16 {
            roi.height = 16;
        }

        if roi.width > 10 || roi.height > 10 {
            center[0] = 199;
        }

        const auto config = ((roi.height - 1) << 4 | (roi.width - 1)) as u8;

        this->write_bytes(VL53L1_ROI_CONFIG__MODE_ROI_CENTRE_SPAD, &center);
        this->write_bytes(
            VL53L1_ROI_CONFIG__USER_ROI_REQUESTED_GLOBAL_XY_SIZE,
            {config, 0},
        );

        return Ok();
    }

    /// Get the currenly set ROI.
    ///
    /// # Arguments
    ///
    auto get_roi() -> Result<ROI, Error> {
        auto config = [0];

        this->read_bytes(
            VL53L1_ROI_CONFIG__USER_ROI_REQUESTED_GLOBAL_XY_SIZE,
            &mut config,
        );

        Ok(ROI {
            width: ((config[0] & 0x0F) + 1) as u16,
            height: (((config[0] >> 4) & 0x0F) + 1) as u16,
        })
    }

    /// Set the new ROI center.
    /// If the new ROI clips out of the border this function does not return an error
    /// but only when ranging is started will an error be returned.
    ///
    /// # Arguments
    ///
    /// * `center` - Tne ROI center to apply.
    auto set_roi_center(center: ROICenter) -> Result<void, Error> {
        this->write_bytes(VL53L1_ROI_CONFIG__USER_ROI_CENTRE_SPAD, &[center.spad])
    }

    /// Get the current ROI center.
    auto get_roi_center() -> Result<ROICenter, Error> {
        auto center = [0];

        this->read_bytes(VL53L1_ROI_CONFIG__MODE_ROI_CENTRE_SPAD, &mut center);

        Ok(ROICenter { spad: center[0] })
    }

    /// Set a signal threshold in kcps. Default is 1024
    ///
    /// # Arguments
    ///
    /// * `threshold` - The signal threshold.
    auto set_signal_threshold(threshold: u16) -> Result<void, Error> {
        this->write_bytes(
            VL53L1_RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS,
            &(threshold >> 3).to_be_bytes(),
        )
    }

    /// Get the currently set signal threshold.
    auto get_signal_threshold() -> Result<u16, Error> {
        auto threshold = std::array<uint8_t, 2>{0, 0};

        this->read_bytes(
            VL53L1_RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS,
            &mut threshold,
        );

        Ok(u16::from_be_bytes(threshold) << 3)
    }

    /// Set a sigma threshold in millimeter. Default is 15.
    ///
    /// # Arguments
    ///
    /// * `threshold` - The sigma threshold.
    auto set_sigma_threshold(threshold: u16) -> Result<void, Error> {
        if threshold > (0xFFFF >> 2) {
            return Err(Error::InvalidSigmaThreshold);
        }

        this->write_bytes(
            VL53L1_RANGE_CONFIG__SIGMA_THRESH,
            &(threshold << 2).to_be_bytes(),
        )
    }

    /// Get the currently set sigma threshold.
    auto get_sigma_threshold() -> Result<u16, Error> {
        auto threshold = std::array<uint8_t, 2>{0, 0};

        this->read_bytes(VL53L1_RANGE_CONFIG__SIGMA_THRESH, &mut threshold);

        Ok(u16::from_be_bytes(threshold) >> 2)
    }

    /// Perform temperature calibration of the sensor.
    /// It is recommended to call this function any time the temperature might have changed by more than 8 degrees Celsius
    /// without sensor ranging activity for an extended period.
    auto calibrate_temperature() -> Result<void, Error> {
        this->write_bytes(
            VL53L1_VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND,
            &0x81u16.to_be_bytes(),
        );
        this->write_bytes(VL53L1_VHV_CONFIG__INIT, &0x92u16.to_be_bytes());

        this->start_ranging();

        while !this->is_data_ready()? {}

        this->clear_interrupt();
        this->stop_ranging();

        this->write_bytes(
            VL53L1_VHV_CONFIG__TIMEOUT_MACROP_LOOP_BOUND,
            &0x09u16.to_be_bytes(),
        );
        this->write_bytes(VL53L1_VHV_CONFIG__INIT, &0u16.to_be_bytes());

        return Ok();
    }

    /// Perform offset calibration.
    /// The function returns the offset value found and sets it as the new offset.
    /// Target reflectance = grey 17%
    ///
    /// # Arguments
    ///
    /// * `target_distance_mm` - Distance to the target in millimeters, ST recommends 100 mm.
    auto calibrate_offset(target_distance_mm: u16) -> Result<i16, Error> {
        this->write_bytes(
            VL53L1_ALGO__PART_TO_PART_RANGE_OFFSET_MM,
            &0u16.to_be_bytes(),
        );
        this->write_bytes(VL53L1_MM_CONFIG__INNER_OFFSET_MM, &0u16.to_be_bytes());
        this->write_bytes(VL53L1_MM_CONFIG__OUTER_OFFSET_MM, &0u16.to_be_bytes());

        this->start_ranging();

        auto average_distance = 0;

        const auto measurement_count = 50;

        for (size_t _ = 0; _ < measurement_count; _++) {
            while this->is_data_ready()? {}

            average_distance += this->get_distance();
            this->clear_interrupt();
        }

        this->stop_ranging();

        average_distance /= measurement_count;

        const auto offset = int16_t(target_distance_mm) - int16_t(average_distance);

        this->set_offset(offset);

        Ok(offset)
    }

    /// Perform crosstalk calibration.
    /// The function returns the crosstalk value found and set it as the new crosstalk correction.
    /// Target reflectance = grey 17%
    ///
    ///  Arguments
    ///
    /// * `target_distance_mm` - Distance to the target in millimeters, ST recommends 100 mm.
    auto calibrate_cross_talk(target_distance_mm: u16) -> Result<u16, Error> {
        this->write_bytes(
            VL53L1_ALGO__CROSSTALK_COMPENSATION_PLANE_OFFSET_KCPS,
            &0u16.to_be_bytes(),
        );

        this->start_ranging();

        auto average_distance = 0;
        auto average_spad_cnt = 0;
        auto average_signal_rate = 0;

        const auto measurement_count = 50;

        for (size_t _ = 0; _ < measurement_count; _++) {
            while this->is_data_ready()? {}

            average_distance += this->get_distance();
            average_signal_rate += this->get_signal_rate();
            this->clear_interrupt();
            average_spad_cnt += this->get_spad_count();
        }

        this->stop_ranging();

        average_distance /= measurement_count;
        average_spad_cnt /= measurement_count;
        average_signal_rate /= measurement_count;

        auto calibrate_val = 512
            * (average_signal_rate as u32
                * (1 - (average_distance as u32 / target_distance_mm as u32)))
            / average_spad_cnt as u32;

        if(calibrate_val > 0xFFFF){
            calibrate_val = 0xFFFF;
        }

        const auto config = ((calibrate_val as u16).saturating_mul(1000) >> 9).to_be_bytes();

        this->write_bytes(
            VL53L1_ALGO__CROSSTALK_COMPENSATION_PLANE_OFFSET_KCPS,
            &config,
        );

        Ok(0)
    }
}

}