/*
 *  linux/drivers/mmc/host/glamo-mmc.c - Glamo MMC driver
 *
 *  Copyright (C) 2007 OpenMoko, Inc,  Andy Green <andy@openmoko.com>
 *  Based on the Glamo MCI driver that was -->
 *
 *  Copyright (C) 2007 OpenMoko, Inc,  Andy Green <andy@openmoko.com>
 *  Based on S3C MMC driver that was:
 *  Copyright (C) 2004-2006 maintech GmbH, Thomas Kleffel <tk@maintech.de>
 *
 *  and
 *
 *  Based on S3C MMC driver that was (original copyright notice ---->)
 *
 * (C) Copyright 2006 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *
 * based on u-boot pxa MMC driver and linux/drivers/mmc/s3c2410mci.c
 * (C) 2005-2005 Thomas Kleffel
 *
 *  Copyright (C) 2004-2006 maintech GmbH, Thomas Kleffel <tk@maintech.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <config.h>
#include <common.h>
#include <mmc.h>
#include <asm/errno.h>
#include <asm/io.h>
#include <part.h>
#include <fat.h>
#include <pcf50633.h>

#include "glamo-regs.h"
#include "glamo-mmc.h"

#if defined(CONFIG_MMC) && defined(CONFIG_MMC_GLAMO)

#define MMC_BLOCK_SIZE_BITS 9
#define MMC_BLOCK_SIZE (1 << MMC_BLOCK_SIZE_BITS)

#define GLAMO_REG(x) (*(volatile u16 *)(CONFIG_GLAMO_BASE + x))
#define GLAMO_INTRAM_OFFSET (8 * 1024 * 1024)
#define GLAMO_FB_SIZE ((8 * 1024 * 1024) - 0x10000)
#define GLAMO_START_OF_MMC_INTMEM ((volatile u16 *)(CONFIG_GLAMO_BASE + \
				  GLAMO_INTRAM_OFFSET + GLAMO_FB_SIZE))

static int ccnt;
static block_dev_desc_t mmc_dev;
static mmc_csd_t mmc_csd;
static int mmc_ready = 0;
static int wide = 0;
static enum card_type card_type = CARDTYPE_NONE;

block_dev_desc_t * mmc_get_dev(int dev)
{
	return (block_dev_desc_t *)&mmc_dev;
}

static void
glamo_reg_write(u_int16_t val, u_int16_t reg)
{
	GLAMO_REG(reg) = val;
}

static u_int16_t
glamo_reg_read(u_int16_t reg)
{
	return GLAMO_REG(reg);
}

unsigned char CRC7(u8 * pu8, int cnt)
{
	u8 crc = 0;

	while (cnt--) {
		int n;
		u8 d = *pu8++;
		for (n = 0; n < 8; n++) {
			crc <<= 1;
			if ((d & 0x80) ^ (crc & 0x80))
				crc ^= 0x09;
			d <<= 1;
		}
	}
	return (crc << 1) | 1;
}

/* MMC_DEFAULT_RCA should probably be just 1, but this may break other code
   that expects it to be shifted. */
static u_int16_t rca = MMC_DEFAULT_RCA >> 16;

static void do_pio_read(u16 *buf, int count_words)
{
	volatile u16 *from_ptr = GLAMO_START_OF_MMC_INTMEM;

	while (count_words--)
		*buf++ = *from_ptr++;
}

static void do_pio_write(u16 *buf, int count_words)
{
	volatile u16 *to_ptr = GLAMO_START_OF_MMC_INTMEM;

	while (count_words--)
		*to_ptr++ = *buf++;
}


static int mmc_cmd(int opcode, int arg, int flags,
		   int data_size, int data_blocks,
		   int will_stop, u16 *resp)
{
	u16 * pu16 = (u16 *)&resp[0];
	u16 * reg_resp = (u16 *)(CONFIG_GLAMO_BASE + GLAMO_REGOFS_MMC +
				 GLAMO_REG_MMC_CMD_RSP1);
	u16 status;
	int n;
	u8 u8a[6];
	u16 fire = 0;
	int cmd_is_stop = 0;
	int error = 0;

#if 0
	printf("mmc_cmd(opcode=%d, arg=0x%08X, flags=0x%x, "
	       "data_size=%d, data_blocks=%d, will_stop=%d, resp=%p)\n",
	       opcode, arg, flags, data_size, data_blocks, will_stop, resp);
#endif
	switch (opcode) {
	case MMC_STOP_TRANSMISSION:
		cmd_is_stop = 1;
		break;
	default:
		break;
	}

	ccnt++;

	 /* this guy has data to read/write? */
	if ((!cmd_is_stop) && (flags & (MMC_DATA_WRITE | MMC_DATA_READ))) {
		/*
		* the S-Media-internal RAM offset for our MMC buffer
		*/
		glamo_reg_write((u16)GLAMO_FB_SIZE,
			GLAMO_REGOFS_MMC + GLAMO_REG_MMC_WDATADS1);
		glamo_reg_write((u16)(GLAMO_FB_SIZE >> 16),
			GLAMO_REGOFS_MMC + GLAMO_REG_MMC_WDATADS2);
		glamo_reg_write((u16)GLAMO_FB_SIZE,
			GLAMO_REGOFS_MMC + GLAMO_REG_MMC_RDATADS1);
		glamo_reg_write((u16)(GLAMO_FB_SIZE >> 16),
			GLAMO_REGOFS_MMC + GLAMO_REG_MMC_RDATADS2);

		/* set up the block info */
		glamo_reg_write(data_size, GLAMO_REGOFS_MMC +
					   GLAMO_REG_MMC_DATBLKLEN);
		glamo_reg_write(data_blocks, GLAMO_REGOFS_MMC +
					     GLAMO_REG_MMC_DATBLKCNT);
	}

	/* if we can't do it, reject as busy */
	if (!glamo_reg_read(GLAMO_REGOFS_MMC + GLAMO_REG_MMC_RB_STAT1) &
	     GLAMO_STAT1_MMC_IDLE)
		return -1;

	/* create an array in wire order for CRC computation */
	u8a[0] = 0x40 | (opcode & 0x3f);
	u8a[1] = (arg >> 24);
	u8a[2] = (arg >> 16);
	u8a[3] = (arg >> 8);
	u8a[4] = arg;
	u8a[5] = CRC7(&u8a[0], 5); /* CRC7 on first 5 bytes of packet */

	/* issue the wire-order array including CRC in register order */
	glamo_reg_write((u8a[4] << 8) | u8a[5],
			GLAMO_REGOFS_MMC + GLAMO_REG_MMC_CMD_REG1);
	glamo_reg_write((u8a[2] << 8) | u8a[3],
			GLAMO_REGOFS_MMC + GLAMO_REG_MMC_CMD_REG2);
	glamo_reg_write((u8a[0] << 8) | u8a[1],
			GLAMO_REGOFS_MMC + GLAMO_REG_MMC_CMD_REG3);

	/* command index toggle */
	fire |= (ccnt & 1) << 12;

	/* set type of command */
	switch (mmc_cmd_type(flags)) {
	case MMC_CMD_BC:
		fire |= GLAMO_FIRE_MMC_CMDT_BNR;
		break;
	case MMC_CMD_BCR:
		fire |= GLAMO_FIRE_MMC_CMDT_BR;
		break;
	case MMC_CMD_AC:
		fire |= GLAMO_FIRE_MMC_CMDT_AND;
		break;
	case MMC_CMD_ADTC:
		fire |= GLAMO_FIRE_MMC_CMDT_AD;
		break;
	}
	/*
	 * if it expects a response, set the type expected
	 *
	 * R1, Length  : 48bit, Normal response
	 * R1b, Length : 48bit, same R1, but added card busy status
	 * R2, Length  : 136bit (really 128 bits with CRC snipped)
	 * R3, Length  : 48bit (OCR register value)
	 * R4, Length  : 48bit, SDIO_OP_CONDITION, Reverse SDIO Card
	 * R5, Length  : 48bit, IO_RW_DIRECTION, Reverse SDIO Card
	 * R6, Length  : 48bit (RCA register)
	 * R7, Length  : 48bit (interface condition, VHS(voltage supplied),
	 *                     check pattern, CRC7)
	 */
	switch (mmc_resp_type(flags)) {
	case MMC_RSP_R6: /* same index as R7 and R1 */
		fire |= GLAMO_FIRE_MMC_RSPT_R1;
		break;
	case MMC_RSP_R1B:
		fire |= GLAMO_FIRE_MMC_RSPT_R1b;
		break;
	case MMC_RSP_R2:
		fire |= GLAMO_FIRE_MMC_RSPT_R2;
		break;
	case MMC_RSP_R3:
		fire |= GLAMO_FIRE_MMC_RSPT_R3;
		break;
	/* R4 and R5 supported by chip not defined in linux/mmc/core.h (sdio) */
	}
	/*
	 * From the command index, set up the command class in the host ctrllr
	 *
	 * missing guys present on chip but couldn't figure out how to use yet:
	 *     0x0 "stream read"
	 *     0x9 "cancel running command"
	 */
	switch (opcode) {
	case MMC_READ_SINGLE_BLOCK:
		fire |= GLAMO_FIRE_MMC_CC_SBR; /* single block read */
		break;
	case MMC_SWITCH: /* 64 byte payload */
	case 0x33: /* observed issued by MCI */
	case MMC_READ_MULTIPLE_BLOCK:
		/* we will get an interrupt off this */
		if (!will_stop)
			/* multiblock no stop */
			fire |= GLAMO_FIRE_MMC_CC_MBRNS;
		else
			 /* multiblock with stop */
			fire |= GLAMO_FIRE_MMC_CC_MBRS;
		break;
	case MMC_WRITE_BLOCK:
		fire |= GLAMO_FIRE_MMC_CC_SBW; /* single block write */
		break;
	case MMC_WRITE_MULTIPLE_BLOCK:
		if (will_stop)
			 /* multiblock with stop */
			fire |= GLAMO_FIRE_MMC_CC_MBWS;
		else
			 /* multiblock NO stop-- 'RESERVED'? */
			fire |= GLAMO_FIRE_MMC_CC_MBWNS;
		break;
	case MMC_STOP_TRANSMISSION:
		fire |= GLAMO_FIRE_MMC_CC_STOP; /* STOP */
		break;
	default:
		fire |= GLAMO_FIRE_MMC_CC_BASIC; /* "basic command" */
		break;
	}
	/* enforce timeout */
	glamo_reg_write(0xfff, GLAMO_REGOFS_MMC + GLAMO_REG_MMC_TIMEOUT);

	/* Generate interrupt on txfer; drive strength max */
	glamo_reg_write((glamo_reg_read(GLAMO_REGOFS_MMC +
					GLAMO_REG_MMC_BASIC) & 0xfe) |
			 0x0800 | GLAMO_BASIC_MMC_NO_CLK_RD_WAIT |
			 GLAMO_BASIC_MMC_EN_COMPL_INT |
			 GLAMO_BASIC_MMC_EN_DR_STR0 |
			 GLAMO_BASIC_MMC_EN_DR_STR1,
			GLAMO_REGOFS_MMC + GLAMO_REG_MMC_BASIC);

	/* send the command out on the wire */
	/* dev_info(&host->pdev->dev, "Using FIRE %04X\n", fire); */
	glamo_reg_write(fire, GLAMO_REGOFS_MMC + GLAMO_REG_MMC_CMD_FIRE);

	/*
	 * we must spin until response is ready or timed out
	 * -- we don't get interrupts unless there is a bulk rx
	 */
	do
		status = glamo_reg_read(GLAMO_REGOFS_MMC +
					GLAMO_REG_MMC_RB_STAT1);
	while ((((status >> 15) & 1) != (ccnt & 1)) ||
		(!(status & (GLAMO_STAT1_MMC_RB_RRDY |
			     GLAMO_STAT1_MMC_RTOUT |
			     GLAMO_STAT1_MMC_DTOUT |
			     GLAMO_STAT1_MMC_BWERR |
			     GLAMO_STAT1_MMC_BRERR))));

	if (status & (GLAMO_STAT1_MMC_RTOUT | GLAMO_STAT1_MMC_DTOUT))
		error = -4;
	if (status & (GLAMO_STAT1_MMC_BWERR | GLAMO_STAT1_MMC_BRERR))
		error = -5;

	if (cmd_is_stop)
		return 0;

	if (error) {
		printf("cmd 0x%x, arg 0x%x flags 0x%x\n", opcode, arg, flags);
		printf("Error after cmd: 0x%x\n", error);
		goto done;
	}
	/*
	 * mangle the response registers in two different exciting
	 * undocumented ways discovered by trial and error
	 */
	if (mmc_resp_type(flags) == MMC_RSP_R2)
		/* grab the response */
		for (n = 0; n < 8; n++) /* super mangle power 1 */
			pu16[n ^ 6] = reg_resp[n];
	else
		for (n = 0; n < 3; n++) /* super mangle power 2 */
			pu16[n] = (reg_resp[n] >> 8) |
				  (reg_resp[n + 1] << 8);
	/*
	 * if we don't have bulk data to take care of, we're done
	 */
	if (!(flags & (MMC_DATA_READ | MMC_DATA_WRITE)))
		goto done;

	/* enforce timeout */
	glamo_reg_write(0xfff, GLAMO_REGOFS_MMC + GLAMO_REG_MMC_TIMEOUT);
	/*
	 * spin
	 */
	while (!(glamo_reg_read(GLAMO_REG_IRQ_STATUS) & GLAMO_IRQ_MMC))
		;
	/* ack this interrupt source */
	glamo_reg_write(GLAMO_IRQ_MMC, GLAMO_REG_IRQ_CLEAR);

	if (status & GLAMO_STAT1_MMC_DTOUT)
		error = -1;
	if (status & (GLAMO_STAT1_MMC_BWERR | GLAMO_STAT1_MMC_BRERR))
		error = -2;
	if (status & GLAMO_STAT1_MMC_RTOUT)
		error = -5;
	if (error) {
		printf("cmd 0x%x, arg 0x%x flags 0x%x\n", opcode, arg, flags);
		printf("Error after resp: 0x%x\n", status);
		goto done;
	}
#if 0
	if (flags & MMC_DATA_READ) {
		volatile u8 * pu8 = (volatile u8 *)GLAMO_START_OF_MMC_INTMEM;
		for (n = 0; n < 512; n += 16) {
			int n1;
			for (n1 = 0; n1 < 16; n1++) {
				printf("%02X ", pu8[n + n1]);
			}
			printf("\n");
		}
	}
#endif
	return 0;

done:
	return error;
}

static void glamo_mci_reset(void)
{
	/* reset MMC controller */
	glamo_reg_write(GLAMO_CLOCK_MMC_RESET | GLAMO_CLOCK_MMC_DG_TCLK |
		   GLAMO_CLOCK_MMC_EN_TCLK | GLAMO_CLOCK_MMC_DG_M9CLK |
		   GLAMO_CLOCK_MMC_EN_M9CLK,
		  GLAMO_REG_CLOCK_MMC);
	udelay(100000);
	/* and disable reset */
	glamo_reg_write(GLAMO_CLOCK_MMC_DG_TCLK |
		   GLAMO_CLOCK_MMC_EN_TCLK | GLAMO_CLOCK_MMC_DG_M9CLK |
		   GLAMO_CLOCK_MMC_EN_M9CLK,
		   GLAMO_REG_CLOCK_MMC);
}


static u_int8_t ldo_voltage(unsigned int millivolts)
{
	if (millivolts < 900)
		return 0;
	else if (millivolts > 3600)
		return 0x1f;

	millivolts -= 900;
	return millivolts / 100;
}

static
int glamo_mmc_read(ulong src, uint blksize, uchar *dst, int size)
{
	int resp;
	u8 response[16];
	int size_original = size;

	if ((!size) || (size & (MMC_BLOCK_SIZE - 1))) {
		printf("Bad size %d\n", size);
		return 0;
	}

	if (((int)dst) & 1) {
		printf("Bad align on dst\n");
		return 0;
	}

	resp = mmc_cmd(MMC_SET_BLOCKLEN, MMC_BLOCK_SIZE,
		       MMC_CMD_AC | MMC_RSP_R1, 0, 0, 0,
		       (u16 *)&response[0]);

	while (size) {
		switch (card_type) {
		case CARDTYPE_SDHC: /* block addressing */
			if (blksize != MMC_BLOCK_SIZE) {
				printf("Bad align on src\n");
				return 0;
			}
			resp = mmc_cmd(MMC_READ_SINGLE_BLOCK,
				       src,
				       MMC_CMD_ADTC | MMC_RSP_R1 |
				       MMC_DATA_READ, MMC_BLOCK_SIZE, 1, 0,
				       (u16 *)&response[0]);
			src += 1;
			break;
		default: /* byte addressing */
			resp = mmc_cmd(MMC_READ_SINGLE_BLOCK, src * blksize,
				MMC_CMD_ADTC | MMC_RSP_R1 | MMC_DATA_READ,
				MMC_BLOCK_SIZE, 1, 0,
				(u16 *)&response[0]);
			src += MMC_BLOCK_SIZE / blksize;
			break;
		}
		do_pio_read((u16 *)dst, MMC_BLOCK_SIZE >> 1);

		if (size >= MMC_BLOCK_SIZE)
			size -= MMC_BLOCK_SIZE;
		else
			size = 0;
		dst += MMC_BLOCK_SIZE;
	}
	return size_original;
}

ulong mmc_bread(int dev_num, ulong blknr, ulong blkcnt, void *dst)
{
	ulong size = blkcnt * MMC_BLOCK_SIZE;

	if (!blkcnt)
		return 0;

/*	printf("mmc_bread(%d, %ld, %ld, %p)\n", dev_num, blknr, blkcnt, dst); */
	glamo_mmc_read(blknr, MMC_BLOCK_SIZE, dst, size);
	return blkcnt;
}

int mmc_read(ulong src, uchar *dst, int size)
{
        if (src & (MMC_BLOCK_SIZE - 1))
                return (glamo_mmc_read (src, 1, dst, size));
        else
                return (glamo_mmc_read (src >> MMC_BLOCK_SIZE_BITS,
                                        MMC_BLOCK_SIZE, dst, size));
}

int mmc_write(uchar *src, ulong dst, int size)
{
	int resp;
	u8 response[16];
	int size_original = size;

	if ((!size) || (size & (MMC_BLOCK_SIZE - 1))) {
		printf("Bad size %d\n", size);
		return 0;
	}

	if (((int)dst) & 1) {
		printf("Bad align on dst\n");
		return 0;
	}

	resp = mmc_cmd(MMC_SET_BLOCKLEN, MMC_BLOCK_SIZE,
		       MMC_CMD_AC | MMC_RSP_R1, 0, 0, 0,
		       (u16 *)&response[0]);

	while (size) {
		do_pio_write((u16 *)src, MMC_BLOCK_SIZE >> 1);
		switch (card_type) {
		case CARDTYPE_SDHC: /* block addressing */
			resp = mmc_cmd(MMC_WRITE_BLOCK,
				       dst >> MMC_BLOCK_SIZE_BITS,
				       MMC_CMD_ADTC | MMC_RSP_R1 |
								MMC_DATA_WRITE,
				       MMC_BLOCK_SIZE, 1, 0,
				       (u16 *)&response[0]);
			break;
		default: /* byte addressing */
			resp = mmc_cmd(MMC_WRITE_BLOCK, dst,
				       MMC_CMD_ADTC | MMC_RSP_R1 |
								MMC_DATA_WRITE,
				       MMC_BLOCK_SIZE, 1, 0,
				       (u16 *)&response[0]);
			break;
		}
		if (size >= MMC_BLOCK_SIZE)
			size -= MMC_BLOCK_SIZE;
		else
			size = 0;
		dst += MMC_BLOCK_SIZE;
		src += MMC_BLOCK_SIZE;
	}
	return size_original;
}

static void print_mmc_cid(mmc_cid_t *cid)
{
	printf("MMC found. Card desciption is:\n");
	printf("Manufacturer ID = %02x%02x%02x\n",
		cid->id[0], cid->id[1], cid->id[2]);
	printf("HW/FW Revision = %x %x\n",cid->hwrev, cid->fwrev);
	cid->hwrev = cid->fwrev = 0;	/* null terminate string */
	printf("Product Name = %s\n",cid->name);
	printf("Serial Number = %02x%02x%02x\n",
		cid->sn[0], cid->sn[1], cid->sn[2]);
	printf("Month = %d\n",cid->month);
	printf("Year = %d\n",1997 + cid->year);
}

static void print_sd_cid(const struct sd_cid *cid)
{
	printf("Card Type:          ");
	switch (card_type) {
	case CARDTYPE_NONE:
		printf("(None)\n");
		break;
	case CARDTYPE_MMC:
		printf("MMC\n");
		break;
	case CARDTYPE_SD:
		printf("SD\n");
		break;
	case CARDTYPE_SD20:
		printf("SD 2.0\n");
		break;
	case CARDTYPE_SDHC:
		printf("SD 2.0 SDHC\n");
		break;
	}
	printf("Manufacturer:       0x%02x, OEM \"%c%c\"\n",
	    cid->mid, cid->oid_0, cid->oid_1);
	printf("Product name:       \"%c%c%c%c%c\", revision %d.%d\n",
	    cid->pnm_0, cid->pnm_1, cid->pnm_2, cid->pnm_3, cid->pnm_4,
	    cid->prv >> 4, cid->prv & 15);
	printf("Serial number:      %u\n",
	    cid->psn_0 << 24 | cid->psn_1 << 16 | cid->psn_2 << 8 |
	    cid->psn_3);
	printf("Manufacturing date: %d/%d\n",
	    cid->mdt_1 & 15,
	    2000+((cid->mdt_0 & 15) << 4)+((cid->mdt_1 & 0xf0) >> 4));
/*	printf("CRC:                0x%02x, b0 = %d\n",
	    cid->crc >> 1, cid->crc & 1); */
}


int mmc_init(int verbose)
{
 	int retries = 14, rc = -ENODEV;
	int resp;
	u8 response[16];
	mmc_cid_t *mmc_cid = (mmc_cid_t *)response;
	struct sd_cid *sd_cid = (struct sd_cid *)response;
	u32 hcs = 0;

	card_type = CARDTYPE_NONE;

	/* enable engine */

	glamo_reg_write(GLAMO_CLOCK_MMC_EN_M9CLK |
			GLAMO_CLOCK_MMC_EN_TCLK |
			GLAMO_CLOCK_MMC_DG_M9CLK |
			GLAMO_CLOCK_MMC_DG_TCLK, GLAMO_REG_CLOCK_MMC);
	glamo_reg_write(glamo_reg_read(GLAMO_REG_HOSTBUS(2)) |
			GLAMO_HOSTBUS2_MMIO_EN_MMC, GLAMO_REG_HOSTBUS(2));

	/* controller reset */

	glamo_mci_reset();

	/* power the sdcard slot */

	pcf50633_reg_write(PCF50633_REG_HCLDOOUT, ldo_voltage(3300));
	udelay(10000);
	pcf50633_reg_write(PCF50633_REG_HCLDOOUT + 1,
		pcf50633_reg_read(PCF50633_REG_HCLDOOUT + 1) | 1); /* on */
	udelay(10000);

	/* start the clock -- slowly (50MHz / 250 == 195kHz */

	glamo_reg_write((glamo_reg_read(GLAMO_REG_CLOCK_GEN8) & 0xff00) | 250,
			 GLAMO_REG_CLOCK_GEN8);

	/* enable clock to divider input */

	glamo_reg_write(glamo_reg_read(
		GLAMO_REG_CLOCK_GEN5_1) | GLAMO_CLOCK_GEN51_EN_DIV_TCLK,
		GLAMO_REG_CLOCK_GEN5_1);

	udelay(100000);

	/* set bus width to 1 */

	glamo_reg_write((glamo_reg_read(GLAMO_REGOFS_MMC +
			 GLAMO_REG_MMC_BASIC) &
			 (~GLAMO_BASIC_MMC_EN_4BIT_DATA)),
					GLAMO_REGOFS_MMC + GLAMO_REG_MMC_BASIC);

	/* reset */

	resp = mmc_cmd(MMC_GO_IDLE_STATE, 0, MMC_CMD_BCR, 0, 0, 0,
		       (u16 *)&response[0]);

	udelay(100000);
	udelay(100000);
	udelay(100000);

	/* SDHC card? */

	resp = mmc_cmd(SD_SEND_IF_COND, 0x000001aa,
		MMC_CMD_BCR | MMC_RSP_R7, 0, 0, 0,
		(u16 *)&response[0]);
	if (!resp && (response[0] == 0xaa)) {
		card_type = CARDTYPE_SD20; /* 2.0 SD, may not be SDHC */
		hcs = 0x40000000;
	}

	/* Well, either way let's say hello in SD card protocol */

	while (retries--) {

		udelay(100000);

		resp = mmc_cmd(MMC_APP_CMD, 0x00000000,
			MMC_CMD_AC | MMC_RSP_R1, 0, 0, 0,
			(u16 *)&response[0]);
		if (resp)
			continue;
		resp = mmc_cmd(SD_APP_OP_COND, hcs | 0x00300000,
			MMC_CMD_BCR | MMC_RSP_R3, 0, 0, 0,
			(u16 *)&response[0]);
		if (resp)
			continue;

		if (response[3] & (1 << 6)) /* asserts block addressing */
			card_type = CARDTYPE_SDHC;

		if (response[3] & (1 << 7)) { /* not busy */
			if (card_type == CARDTYPE_NONE)
				card_type = CARDTYPE_SD;
			break;
		}
	}
	if (retries < 0)
		return 1;

	if (card_type == CARDTYPE_NONE) {
		retries = 10;
		printf("failed to detect SD Card, trying MMC\n");
		do {
			resp = mmc_cmd(MMC_SEND_OP_COND, 0x00ffc000,
				       MMC_CMD_BCR | MMC_RSP_R3, 0, 0, 0,
				       (u16 *)&response[0]);
			debug("resp %x %x\n", response[0], response[1]);
			udelay(50);
		} while (retries-- && !(response[3] & 0x80));
		if (retries >= 0)
			card_type = CARDTYPE_MMC;
		else
			return 1;
	}

	/* fill in device description */
	mmc_dev.if_type = IF_TYPE_MMC;
	mmc_dev.part_type = PART_TYPE_DOS;
	mmc_dev.dev = 0;
	mmc_dev.lun = 0;
	mmc_dev.type = 0;
	mmc_dev.removable = 0;
	mmc_dev.block_read = mmc_bread;
	mmc_dev.blksz = 512;
	mmc_dev.lba = 1 << 16; /* 64K x 512 blocks = 32MB default */

	/* try to get card id */
	resp = mmc_cmd(MMC_ALL_SEND_CID, hcs,
			MMC_CMD_BCR | MMC_RSP_R2, 0, 0, 0,
			(u16 *)&response[0]);
	if (resp)
		return 1;

	switch (card_type) {
	case CARDTYPE_MMC:
		/* TODO configure mmc driver depending on card
			attributes */

		if (verbose)
			print_mmc_cid(mmc_cid);
		sprintf((char *) mmc_dev.vendor,
			"Man %02x%02x%02x Snr %02x%02x%02x",
			mmc_cid->id[0], mmc_cid->id[1], mmc_cid->id[2],
			mmc_cid->sn[0], mmc_cid->sn[1], mmc_cid->sn[2]);
		sprintf((char *) mmc_dev.product, "%s", mmc_cid->name);
		sprintf((char *) mmc_dev.revision, "%x %x",
			mmc_cid->hwrev, mmc_cid->fwrev);

		/* MMC exists, get CSD too */
		resp = mmc_cmd(MMC_SET_RELATIVE_ADDR, MMC_DEFAULT_RCA,
				MMC_CMD_AC | MMC_RSP_R1, 0, 0, 0,
				(u16 *)&response[0]);
		break;

	case CARDTYPE_SD:
	case CARDTYPE_SD20:
	case CARDTYPE_SDHC:
		if (verbose)
			print_sd_cid(sd_cid);
		sprintf((char *) mmc_dev.vendor,
			"Man %02 OEM %c%c \"%c%c%c%c%c\"",
			sd_cid->mid, sd_cid->oid_0, sd_cid->oid_1,
			sd_cid->pnm_0, sd_cid->pnm_1, sd_cid->pnm_2,
			sd_cid->pnm_3, sd_cid->pnm_4);
		sprintf((char *) mmc_dev.product, "%d",
			sd_cid->psn_0 << 24 | sd_cid->psn_1 << 16 |
			sd_cid->psn_2 << 8 | sd_cid->psn_3);
		sprintf((char *) mmc_dev.revision, "%d.%d",
			sd_cid->prv >> 4, sd_cid->prv & 15);

		resp = mmc_cmd(SD_SEND_RELATIVE_ADDR, MMC_DEFAULT_RCA,
				MMC_CMD_BCR | MMC_RSP_R6, 0, 0, 0,
				(u16 *)&response[0]);
		rca = response[2] | (response[3] << 8);
		break;

	default:
		return 1;
	}

	/* grab the CSD */

	resp = mmc_cmd(MMC_SEND_CSD, rca << 16,
			MMC_CMD_AC | MMC_RSP_R2, 0, 0, 0,
			(u16 *)&response[0]);
	if (!resp) {
		mmc_csd_t *csd = (mmc_csd_t *)response;

		memcpy(&mmc_csd, csd, sizeof(csd));
		rc = 0;
		mmc_ready = 1;
		/* FIXME add verbose printout for csd */
		/* printf("READ_BL_LEN=%u, C_SIZE_MULT=%u, C_SIZE=%u\n",
			csd->read_bl_len, csd->c_size_mult1,
			csd->c_size); */
		mmc_dev.blksz = 512;
		mmc_dev.lba = (((unsigned long)1 << csd->c_size_mult1) *
				(unsigned long)csd->c_size) >> 9;
		printf("MMC/SD size:        %dMiB\n", mmc_dev.lba >> 1);
	}

	resp = mmc_cmd(MMC_SELECT_CARD, rca<<16, MMC_CMD_AC | MMC_RSP_R1,
		       0, 0, 0, (u16 *)&response[0]);
	if (resp)
		return 1;

#ifdef CONFIG_MMC_WIDE
	/* yay 4-bit! */
	if (card_type == CARDTYPE_SD || card_type == CARDTYPE_SDHC) {
		resp = mmc_cmd(MMC_APP_CMD, rca<<16, MMC_CMD_AC | MMC_RSP_R1,
		       0, 0, 0, (u16 *)&response[0]);
		resp = mmc_cmd(MMC_SWITCH, 0x02, MMC_CMD_AC | MMC_RSP_R1B,
		       0, 0, 0, (u16 *)&response[0]);
		wide = 1;
		glamo_reg_write(glamo_reg_read(GLAMO_REGOFS_MMC +
			 GLAMO_REG_MMC_BASIC) | GLAMO_BASIC_MMC_EN_4BIT_DATA,
					GLAMO_REGOFS_MMC + GLAMO_REG_MMC_BASIC);
	}
#endif

	/* crank the clock to the final speed, 16MHz */

	glamo_reg_write((glamo_reg_read(GLAMO_REG_CLOCK_GEN8) & 0xff00) | 2,
			 GLAMO_REG_CLOCK_GEN8);

	fat_register_device(&mmc_dev, 1); /* partitions start counting with 1 */

	return rc;
}

void mmc_depower(void)
{
	u8 response[16];

	/* reset */
	mmc_cmd(MMC_GO_IDLE_STATE, 0, MMC_CMD_BCR, 0, 0, 0,
		(u16 *)&response[0]);

	/* hold engine reset, remove clocks */

	glamo_reg_write(GLAMO_CLOCK_MMC_RESET, GLAMO_REG_CLOCK_MMC);

	/* disable engine */

	glamo_reg_write(0, GLAMO_REG_CLOCK_MMC);
	glamo_reg_write(glamo_reg_read(GLAMO_REG_HOSTBUS(2)) &
			(~GLAMO_HOSTBUS2_MMIO_EN_MMC), GLAMO_REG_HOSTBUS(2));

	/* remove power */

	pcf50633_reg_write(PCF50633_REG_HCLDOOUT + 1,
		pcf50633_reg_read(PCF50633_REG_HCLDOOUT + 1) & ~1); /* off */
}

int
mmc_ident(block_dev_desc_t *dev)
{
	return 0;
}

int
mmc2info(ulong addr)
{
	/* FIXME hard codes to 32 MB device */
	if (addr >= CFG_MMC_BASE && addr < CFG_MMC_BASE + 0x02000000)
		return 1;

	return 0;
}


#endif /* defined(CONFIG_MMC) && defined(CONFIG_MMC_GLAMO) */
