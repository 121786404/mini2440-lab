/* u-boot driver for the tpo JBT6K74-AS LCM ASIC
 *
 * Copyright (C) 2006-2007 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */
#include <common.h>
#include <spi.h>
#include <video_fb.h>
#include <asm/errno.h>
#include <s3c2410.h>
#include "jbt6k74.h"

#if 0
#define DEBUGP(x, args...) printf("%s: " x, __FUNCTION__, ## args);
#define DEBUGPC(x, args...) printf(x, ## args);
#else
#define DEBUGP(x, args...) do { } while (0)
#define DEBUGPC(x, args...) do { } while (0)
#endif


enum jbt_register {
	JBT_REG_SLEEP_IN		= 0x10,
	JBT_REG_SLEEP_OUT		= 0x11,

	JBT_REG_DISPLAY_OFF		= 0x28,
	JBT_REG_DISPLAY_ON		= 0x29,

	JBT_REG_RGB_FORMAT		= 0x3a,
	JBT_REG_QUAD_RATE		= 0x3b,

	JBT_REG_POWER_ON_OFF		= 0xb0,
	JBT_REG_BOOSTER_OP		= 0xb1,
	JBT_REG_BOOSTER_MODE		= 0xb2,
	JBT_REG_BOOSTER_FREQ		= 0xb3,
	JBT_REG_OPAMP_SYSCLK		= 0xb4,
	JBT_REG_VSC_VOLTAGE		= 0xb5,
	JBT_REG_VCOM_VOLTAGE		= 0xb6,
	JBT_REG_EXT_DISPL		= 0xb7,
	JBT_REG_OUTPUT_CONTROL		= 0xb8,
	JBT_REG_DCCLK_DCEV		= 0xb9,
	JBT_REG_DISPLAY_MODE1		= 0xba,
	JBT_REG_DISPLAY_MODE2		= 0xbb,
	JBT_REG_DISPLAY_MODE		= 0xbc,
	JBT_REG_ASW_SLEW		= 0xbd,
	JBT_REG_DUMMY_DISPLAY		= 0xbe,
	JBT_REG_DRIVE_SYSTEM		= 0xbf,

	JBT_REG_SLEEP_OUT_FR_A		= 0xc0,
	JBT_REG_SLEEP_OUT_FR_B		= 0xc1,
	JBT_REG_SLEEP_OUT_FR_C		= 0xc2,
	JBT_REG_SLEEP_IN_LCCNT_D	= 0xc3,
	JBT_REG_SLEEP_IN_LCCNT_E	= 0xc4,
	JBT_REG_SLEEP_IN_LCCNT_F	= 0xc5,
	JBT_REG_SLEEP_IN_LCCNT_G	= 0xc6,

	JBT_REG_GAMMA1_FINE_1		= 0xc7,
	JBT_REG_GAMMA1_FINE_2		= 0xc8,
	JBT_REG_GAMMA1_INCLINATION	= 0xc9,
	JBT_REG_GAMMA1_BLUE_OFFSET	= 0xca,

	JBT_REG_BLANK_CONTROL		= 0xcf,
	JBT_REG_BLANK_TH_TV		= 0xd0,
	JBT_REG_CKV_ON_OFF		= 0xd1,
	JBT_REG_CKV_1_2			= 0xd2,
	JBT_REG_OEV_TIMING		= 0xd3,
	JBT_REG_ASW_TIMING_1		= 0xd4,
	JBT_REG_ASW_TIMING_2		= 0xd5,

	JBT_REG_HCLOCK_VGA		= 0xec,
	JBT_REG_HCLOCK_QVGA		= 0xed,

};

static const char *jbt_state_names[] = {
	[JBT_STATE_DEEP_STANDBY]	= "deep-standby",
	[JBT_STATE_SLEEP]		= "sleep",
	[JBT_STATE_NORMAL]		= "normal",
};

#ifndef CONFIG_GTA02_REVISION

#define GTA01_SCLK	(1 << 7) 	/* GPG7 */
#define GTA01_MOSI	(1 << 6)	/* GPG6 */
#define GTA01_MISO	(1 << 5)	/* GPG5 */
#define GTA01_CS	(1 << 3)	/* GPG3 */

#define SPI_READ        ((immr->GPGDAT & GTA01_MISO) != 0)

#define SPI_CS(bit) 	if (bit) gpio->GPGDAT |= GTA01_CS; \
			else gpio->GPGDAT &= ~GTA01_CS

#define SPI_SDA(bit)    if (bit) gpio->GPGDAT |=  GTA01_MOSI; \
			else    gpio->GPGDAT &= ~GTA01_MOSI

#define SPI_SCL(bit)    if (bit) gpio->GPGDAT |=  GTA01_SCLK; \
			else    gpio->GPGDAT &= ~GTA01_SCLK

#else /* GTA02 */

extern void smedia3362_spi_cs(int);
extern void smedia3362_spi_sda(int);
extern void smedia3362_spi_scl(int);
extern void smedia3362_lcm_reset(int);

#define SPI_CS(b)   smedia3362_spi_cs(b)
#define SPI_SDA(b)  smedia3362_spi_sda(b)
#define SPI_SCL(b)  smedia3362_spi_scl(b)

#endif


/* 150uS minimum clock cycle, we have two of this plus our other
 * instructions */
#define SPI_DELAY	udelay(100)	/* 200uS */


#define JBT_TX_BUF_SIZE
struct jbt_info {
	enum jbt_state state;
	u_int16_t tx_buf[4];
	struct spi_device *spi_dev;
};

static struct jbt_info _jbt, *jbt = &_jbt;

static int jbt_spi_xfer(int wordnum, int bitlen, u_int16_t *dout)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();
	u_int16_t tmpdout = 0;
	int   i, j;

	DEBUGP("spi_xfer: dout %08X wordnum %u bitlen %d\n",
		*(uint *)dout, wordnum, bitlen);

	SPI_CS(0);

	for (i = 0; i < wordnum; i ++) {
		tmpdout = dout[i];

		for (j = 0; j < bitlen; j++) {
			SPI_SCL(0);
			if (tmpdout & (1 << bitlen-1)) {
				SPI_SDA(1);
				DEBUGPC("1");
			} else {
				SPI_SDA(0);
				DEBUGPC("0");
			}
			SPI_DELAY;
			SPI_SCL(1);
			SPI_DELAY;
			tmpdout <<= 1;
		}
		DEBUGPC(" ");
	}
	DEBUGPC("\n");

	SPI_CS(1);

	return 0;
}

#define JBT_COMMAND	0x000
#define JBT_DATA	0x100

static int jbt_reg_write_nodata(struct jbt_info *jbt, u_int8_t reg)
{
	int rc;

	jbt->tx_buf[0] = JBT_COMMAND | reg;

	rc = jbt_spi_xfer(1, 9, jbt->tx_buf);

	return rc;
}


static int jbt_reg_write(struct jbt_info *jbt, u_int8_t reg, u_int8_t data)
{
	int rc;

	jbt->tx_buf[0] = JBT_COMMAND | reg;
	jbt->tx_buf[1] = JBT_DATA | data;

	rc = jbt_spi_xfer(2, 9, jbt->tx_buf);

	return rc;
}

static int jbt_reg_write16(struct jbt_info *jbt, u_int8_t reg, u_int16_t data)
{
	int rc;

	jbt->tx_buf[0] = JBT_COMMAND | reg;
	jbt->tx_buf[1] = JBT_DATA | (data >> 8);
	jbt->tx_buf[2] = JBT_DATA | (data & 0xff);

	rc = jbt_spi_xfer(3, 9, jbt->tx_buf);

	return rc;
}

static int jbt_init_regs(struct jbt_info *jbt)
{
	int rc;

	DEBUGP("entering\n");

	rc = jbt_reg_write(jbt, JBT_REG_DISPLAY_MODE1, 0x01);
	rc |= jbt_reg_write(jbt, JBT_REG_DISPLAY_MODE2, 0x00);
	rc |= jbt_reg_write(jbt, JBT_REG_RGB_FORMAT, 0x60);
	rc |= jbt_reg_write(jbt, JBT_REG_DRIVE_SYSTEM, 0x10);
	rc |= jbt_reg_write(jbt, JBT_REG_BOOSTER_OP, 0x56);
	rc |= jbt_reg_write(jbt, JBT_REG_BOOSTER_MODE, 0x33);
	rc |= jbt_reg_write(jbt, JBT_REG_BOOSTER_FREQ, 0x11);
	rc |= jbt_reg_write(jbt, JBT_REG_BOOSTER_FREQ, 0x11);
	rc |= jbt_reg_write(jbt, JBT_REG_OPAMP_SYSCLK, 0x02);
	rc |= jbt_reg_write(jbt, JBT_REG_VSC_VOLTAGE, 0x2b);
	rc |= jbt_reg_write(jbt, JBT_REG_VCOM_VOLTAGE, 0x40);
	rc |= jbt_reg_write(jbt, JBT_REG_EXT_DISPL, 0x03);
	rc |= jbt_reg_write(jbt, JBT_REG_DCCLK_DCEV, 0x04);
	/*
	 * default of 0x02 in JBT_REG_ASW_SLEW responsible for 72Hz requirement
	 * to avoid red / blue flicker
	 */
	rc |= jbt_reg_write(jbt, JBT_REG_ASW_SLEW, 0x04);
	rc |= jbt_reg_write(jbt, JBT_REG_DUMMY_DISPLAY, 0x00);

	rc |= jbt_reg_write(jbt, JBT_REG_SLEEP_OUT_FR_A, 0x11);
	rc |= jbt_reg_write(jbt, JBT_REG_SLEEP_OUT_FR_B, 0x11);
	rc |= jbt_reg_write(jbt, JBT_REG_SLEEP_OUT_FR_C, 0x11);
	rc |= jbt_reg_write16(jbt, JBT_REG_SLEEP_IN_LCCNT_D, 0x2040);
	rc |= jbt_reg_write16(jbt, JBT_REG_SLEEP_IN_LCCNT_E, 0x60c0);
	rc |= jbt_reg_write16(jbt, JBT_REG_SLEEP_IN_LCCNT_F, 0x1020);
	rc |= jbt_reg_write16(jbt, JBT_REG_SLEEP_IN_LCCNT_G, 0x60c0);

	rc |= jbt_reg_write16(jbt, JBT_REG_GAMMA1_FINE_1, 0x5533);
	rc |= jbt_reg_write(jbt, JBT_REG_GAMMA1_FINE_2, 0x00);
	rc |= jbt_reg_write(jbt, JBT_REG_GAMMA1_INCLINATION, 0x00);
	rc |= jbt_reg_write(jbt, JBT_REG_GAMMA1_BLUE_OFFSET, 0x00);
	rc |= jbt_reg_write(jbt, JBT_REG_GAMMA1_BLUE_OFFSET, 0x00);

	rc |= jbt_reg_write16(jbt, JBT_REG_HCLOCK_VGA, 0x1f0);
	rc |= jbt_reg_write(jbt, JBT_REG_BLANK_CONTROL, 0x02);
	rc |= jbt_reg_write16(jbt, JBT_REG_BLANK_TH_TV, 0x0804);
	rc |= jbt_reg_write16(jbt, JBT_REG_BLANK_TH_TV, 0x0804);

	rc |= jbt_reg_write(jbt, JBT_REG_CKV_ON_OFF, 0x01);
	rc |= jbt_reg_write16(jbt, JBT_REG_CKV_1_2, 0x0000);

	rc |= jbt_reg_write16(jbt, JBT_REG_OEV_TIMING, 0x0d0e);
	rc |= jbt_reg_write16(jbt, JBT_REG_ASW_TIMING_1, 0x11a4);
	rc |= jbt_reg_write(jbt, JBT_REG_ASW_TIMING_2, 0x0e);

#if 0
	rc |= jbt_reg_write16(jbt, JBT_REG_HCLOCK_QVGA, 0x00ff);
	rc |= jbt_reg_write16(jbt, JBT_REG_HCLOCK_QVGA, 0x00ff);
#endif

	return rc;
}

static int standby_to_sleep(struct jbt_info *jbt)
{
	int rc;

	DEBUGP("entering\n");

	/* three times command zero */
	rc = jbt_reg_write_nodata(jbt, 0x00);
	udelay(1000);
	rc = jbt_reg_write_nodata(jbt, 0x00);
	udelay(1000);
	rc = jbt_reg_write_nodata(jbt, 0x00);
	udelay(1000);

	/* deep standby out */
	rc |= jbt_reg_write(jbt, JBT_REG_POWER_ON_OFF, 0x17);

	return rc;
}

static int sleep_to_normal(struct jbt_info *jbt)
{
	int rc;
	DEBUGP("entering\n");

	/* RGB I/F on, RAM wirte off, QVGA through, SIGCON enable */
	rc = jbt_reg_write(jbt, JBT_REG_DISPLAY_MODE, 0x80);

	/* Quad mode off */
	rc |= jbt_reg_write(jbt, JBT_REG_QUAD_RATE, 0x00);

	/* AVDD on, XVDD on */
	rc |= jbt_reg_write(jbt, JBT_REG_POWER_ON_OFF, 0x16);

	/* Output control */
	rc |= jbt_reg_write16(jbt, JBT_REG_OUTPUT_CONTROL, 0xfff9);

	/* Sleep mode off */
	rc |= jbt_reg_write_nodata(jbt, JBT_REG_SLEEP_OUT);

	/* at this point we have like 50% grey */

	/* initialize register set */
	rc |= jbt_init_regs(jbt);
	return rc;
}

static int normal_to_sleep(struct jbt_info *jbt)
{
	int rc;
	DEBUGP("entering\n");

	rc = jbt_reg_write_nodata(jbt, JBT_REG_DISPLAY_OFF);
	rc |= jbt_reg_write16(jbt, JBT_REG_OUTPUT_CONTROL, 0x8002);
	rc |= jbt_reg_write_nodata(jbt, JBT_REG_SLEEP_IN);

	return rc;
}

static int sleep_to_standby(struct jbt_info *jbt)
{
	DEBUGP("entering\n");
	return jbt_reg_write(jbt, JBT_REG_POWER_ON_OFF, 0x00);
}

/* frontend function */
int jbt6k74_enter_state(enum jbt_state new_state)
{
	int rc = -EINVAL;

	DEBUGP("entering(old_state=%u, new_state=%u)\n", jbt->state, new_state);

	switch (jbt->state) {
	case JBT_STATE_DEEP_STANDBY:
		switch (new_state) {
		case JBT_STATE_DEEP_STANDBY:
			rc = 0;
			break;
		case JBT_STATE_SLEEP:
			rc = standby_to_sleep(jbt);
			break;
		case JBT_STATE_NORMAL:
			/* first transition into sleep */
			rc = standby_to_sleep(jbt);
			/* then transition into normal */
			rc |= sleep_to_normal(jbt);
			break;
		}
		break;
	case JBT_STATE_SLEEP:
		switch (new_state) {
		case JBT_STATE_SLEEP:
			rc = 0;
			break;
		case JBT_STATE_DEEP_STANDBY:
			rc = sleep_to_standby(jbt);
			break;
		case JBT_STATE_NORMAL:
			rc = sleep_to_normal(jbt);
			break;
		}
		break;
	case JBT_STATE_NORMAL:
		switch (new_state) {
		case JBT_STATE_NORMAL:
			rc = 0;
			break;
		case JBT_STATE_DEEP_STANDBY:
			/* first transition into sleep */
			rc = normal_to_sleep(jbt);
			/* then transition into deep standby */
			rc |= sleep_to_standby(jbt);
			break;
		case JBT_STATE_SLEEP:
			rc = normal_to_sleep(jbt);
			break;
		}
		break;
	}

	return rc;
}

int jbt6k74_display_onoff(int on)
{
	DEBUGP("entering\n");
	if (on)
		return jbt_reg_write_nodata(jbt, JBT_REG_DISPLAY_ON);
	else
		return jbt_reg_write_nodata(jbt, JBT_REG_DISPLAY_OFF);
}

int jbt6k74_init(void)
{
	S3C24X0_GPIO * const gpio = S3C24X0_GetBase_GPIO();

#ifndef CONFIG_GTA02_REVISION
	/* initialize SPI for GPIO bitbang */
	gpio->GPGCON &= 0xffff033f;
	gpio->GPGCON |= 0x00005440;

	/* get LCM out of reset */
	gpio->GPCDAT |= (1 << 6);
#else /* GTA02 */
	smedia3362_lcm_reset(1);
#endif
	/* according to data sheet: wait 50ms (Tpos of LCM). However, 50ms
	 * seems unreliable with later LCM batches, increasing to 90ms */
	udelay(90000);

	return 0;
}

void board_video_init(GraphicDevice *pGD)
{
	S3C24X0_LCD * const lcd = S3C24X0_GetBase_LCD();

	lcd->LCDCON1 = 0x00000178; /* CLKVAL=1, BPPMODE=16bpp, TFT, ENVID=0 */

	lcd->LCDCON2 = 0x019fc3c1;
	lcd->LCDCON3 = 0x0039df67;
	lcd->LCDCON4 = 0x00000007;
	lcd->LCDCON5 = 0x0001cf09;
	lcd->LPCSEL  = 0x00000000;
}
