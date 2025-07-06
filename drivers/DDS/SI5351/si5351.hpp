/*
 * si5351.h - Si5351 library for Arduino
 *
 * Copyright (C) 2015 - 2019 Jason Milldrum <milldrum@gmail.com>
 *                           Dana H. Myers <k6jq@comcast.net>
 *
 * Many defines derived from clk-si5351.h in the Linux kernel.
 * Sebastian Hesselbarth <sebastian.hesselbarth@gmail.com>
 * Rabeeh Khoury <rabeeh@solid-run.com>
 *
 * do_div() macro derived from /include/asm-generic/div64.h in
 * the Linux kernel.
 * Copyright (C) 2003 Bernardo Innocenti <bernie@develer.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "core/io/regs.hpp"

#include "hal/bus/i2c/i2cdrv.hpp"
#include "hal/bus/spi/spidrv.hpp"

namespace ymd::drivers{
class Si5351{
public:
    // scexpr uint8_t default_addr = 0x60;
    static constexpr auto DEFAULT_I2C_ADDR = hal::I2cSlaveAddr<7>::from_u8(0x60); 

    enum si5351_clock {SI5351_CLK0, SI5351_CLK1, SI5351_CLK2, SI5351_CLK3,
        SI5351_CLK4, SI5351_CLK5, SI5351_CLK6, SI5351_CLK7};

    enum si5351_pll {SI5351_PLLA, SI5351_PLLB};

    enum si5351_drive {SI5351_DRIVE_2MA, SI5351_DRIVE_4MA, SI5351_DRIVE_6MA, SI5351_DRIVE_8MA};

    enum si5351_clock_source {SI5351_CLK_SRC_XTAL, SI5351_CLK_SRC_CLKIN, SI5351_CLK_SRC_MS0, SI5351_CLK_SRC_MS};

    enum si5351_clock_disable {SI5351_CLK_DISABLE_LOW, SI5351_CLK_DISABLE_HIGH, SI5351_CLK_DISABLE_HI_Z, SI5351_CLK_DISABLE_NEVER};

    enum si5351_clock_fanout {SI5351_FANOUT_CLKIN, SI5351_FANOUT_XO, SI5351_FANOUT_MS};

    enum si5351_pll_input {SI5351_PLL_INPUT_XO, SI5351_PLL_INPUT_CLKIN};

    /* Struct definitions */

    struct Si5351RegSet
    {
        uint32_t p1;
        uint32_t p2;
        uint32_t p3;
    };

    struct Si5351Status
    {
        uint8_t SYS_INIT;
        uint8_t LOL_B;
        uint8_t LOL_A;
        uint8_t LOS;
        uint8_t REVID;
    };

    struct Si5351IntStatus
    {
        uint8_t SYS_INIT_STKY;
        uint8_t LOL_B_STKY;
        uint8_t LOL_A_STKY;
        uint8_t LOS_STKY;
    };
private:
	int32_t ref_correction[2];
    uint8_t clkin_div;
    hal::I2cDrv i2c_drv_;
    bool clk_first_set[8];

	hal::HalResult si5351_write_bulk(uint8_t, uint8_t, const uint8_t *);
	hal::HalResult si5351_write(uint8_t, uint8_t);
	hal::HalResult si5351_read(uint8_t, uint8_t & );
public:
    Si5351(const hal::I2cDrv & i2c_drv):i2c_drv_(i2c_drv){;}
    Si5351(hal::I2cDrv && i2c_drv):i2c_drv_(i2c_drv){;}
    Si5351(Some<hal::I2c *> i2c, const hal::I2cSlaveAddr<7> addr = DEFAULT_I2C_ADDR):
        i2c_drv_(hal::I2cDrv(i2c, addr)){};

	bool init(uint8_t, uint32_t, int32_t);
	void reset(void);
	uint8_t set_freq(uint64_t, enum si5351_clock);
	uint8_t set_freq_manual(uint64_t, uint64_t, enum si5351_clock);
	void set_pll(uint64_t, enum si5351_pll);
	void set_ms(enum si5351_clock, struct Si5351RegSet, uint8_t, uint8_t, uint8_t);
	void output_enable(enum si5351_clock, uint8_t);
	void drive_strength(enum si5351_clock, enum si5351_drive);
	void update_status(void);
	void set_correction(int32_t, enum si5351_pll_input);
	void set_phase(enum si5351_clock, uint8_t);
	int32_t get_correction(enum si5351_pll_input);
	void pll_reset(enum si5351_pll);
	void set_ms_source(enum si5351_clock, enum si5351_pll);
	void set_int(enum si5351_clock, uint8_t);
	void set_clock_pwr(enum si5351_clock, uint8_t);
	void set_clock_invert(enum si5351_clock, uint8_t);
	void set_clock_source(enum si5351_clock, enum si5351_clock_source);
	void set_clock_disable(enum si5351_clock, enum si5351_clock_disable);
	void set_clock_fanout(enum si5351_clock_fanout, uint8_t);
	void set_pll_input(enum si5351_pll, enum si5351_pll_input);
	void set_vcxo(uint64_t, uint8_t);
    void set_ref_freq(uint32_t, enum si5351_pll_input);

	struct Si5351Status dev_status = {.SYS_INIT = 0, .LOL_B = 0, .LOL_A = 0,
    .LOS = 0, .REVID = 0};
	struct Si5351IntStatus dev_int_status = {.SYS_INIT_STKY = 0, .LOL_B_STKY = 0,
    .LOL_A_STKY = 0, .LOS_STKY = 0};
	enum si5351_pll pll_assignment[8];
	uint64_t clk_freq[8];
	uint64_t plla_freq;
	uint64_t pllb_freq;
    enum si5351_pll_input plla_ref_osc;
    enum si5351_pll_input pllb_ref_osc;
	uint32_t xtal_freq[2];
private:
	uint64_t pll_calc(enum si5351_pll, uint64_t, struct Si5351RegSet *, int32_t, uint8_t);
	uint64_t multisynth_calc(uint64_t, uint64_t, struct Si5351RegSet *);
	uint64_t multisynth67_calc(uint64_t, uint64_t, struct Si5351RegSet *);
	void update_sys_status(struct Si5351Status *);
	void update_int_status(struct Si5351IntStatus *);
	void ms_div(enum si5351_clock, uint8_t, uint8_t);
	uint8_t select_r_div(uint64_t *);
	uint8_t select_r_div_ms67(uint64_t *);

};

};