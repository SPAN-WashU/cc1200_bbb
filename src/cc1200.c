 /*
 * CC1200 Lib
 *
 * Copyright (C) 2016 University of Utah
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Written by:
 * Anh Luong <luong@eng.utah.edu>
 */

#include "cc1200.h"
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>

/* Defines and register set */
#include "cc1200-const.h"
#include "cc1200-rf-cfg.h"

/******************************************************************************
 * Local Macro Declarations                                                    * 
 ******************************************************************************/
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

// CC1200 SPI
static uint32_t mode = 0;
static uint8_t bits = 8;
static uint32_t speed = 7700000;
static uint16_t delay = 1;

static int fd;

static uint8_t buf[1024];

int cc1200_cmd_strobe(uint8_t cmd)
{
	struct spi_ioc_transfer transfer = {
		.tx_buf = (unsigned long)buf,
		.rx_buf = (unsigned long)buf,
		.len = 0,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	buf[transfer.len++] = cmd;

	// send the SPI message (all of the above fields, inc. buffers)
	return ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
}

int
cc1200_get_status(uint8_t *status)
{
	int ret;
	struct spi_ioc_transfer transfer = {
		.tx_buf = (unsigned long)buf,
		.rx_buf = (unsigned long)buf,
		.len = 0,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	buf[transfer.len++] = CC1200_SNOP;

	// send the SPI message (all of the above fields, inc. buffers)
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
	if(ret < 0)
		return ret;
	else
		*status = buf[0];
	return ret;
}

int cc1200_write_register(uint16_t reg, uint8_t value)
{
	int ret;
	struct spi_ioc_transfer transfer = {
		.tx_buf = (unsigned long)buf,
		.rx_buf = (unsigned long)buf,
		.len = 0,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	// Reg
	if (!CC1200_IS_EXTENDED_ADDR(reg)) {
		buf[transfer.len++] = CC1200_WRITE_BIT | reg;
		buf[transfer.len++] = value;
	} 
	// Extended Address
	else {
		buf[transfer.len++] = CC1200_WRITE_BIT | CC1200_EXT_REG_MASK;
		buf[transfer.len++] = CC1200_UNEXTEND_ADDR(reg);
		buf[transfer.len++] = value;
	}

	// send the SPI message (all of the above fields, inc. buffers)
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
	
	return ret;
}

void cc1200_write_reg_settings(const registerSetting_t *reg_settings,
		uint16_t sizeof_reg_settings)
{
	int i = sizeof_reg_settings / sizeof(registerSetting_t);

	if(reg_settings != NULL) {
		while(i--) {
			cc1200_write_register(reg_settings->addr,
					reg_settings->val);
			reg_settings++;
		}
	}
}

int cc1200_read_register(uint16_t reg, uint8_t *data)
{
	int ret;
	struct spi_ioc_transfer transfer = {
		.tx_buf = (unsigned long)buf,
		.rx_buf = (unsigned long)buf,
		.len = 0,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	// Reg
	if (!CC1200_IS_EXTENDED_ADDR(reg)) {
		buf[transfer.len++] = CC1200_READ_BIT | reg;
	} 
	// Extended Address
	else {
		buf[transfer.len++] = CC1200_READ_BIT | CC1200_EXT_REG_MASK;
		buf[transfer.len++] = CC1200_UNEXTEND_ADDR(reg);
	}

	buf[transfer.len++] = CC1200_SNOP;

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
	if(ret < 0)
		return ret;
	else
		*data = buf[transfer.len-1];

	// send the SPI message (all of the above fields, inc. buffers)
	return ret;
}

int cc1200_write_txfifo(uint8_t *data, uint8_t len)
{
	int ret;
	struct spi_ioc_transfer transfer = {
		.tx_buf = (unsigned long)buf,
		.rx_buf = (unsigned long)buf,
		.len = 0,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	buf[transfer.len++] = CC1200_FIFO | CC1200_WRITE_BIT | CC1200_BURST_BIT;
	transfer.len += len;

	int j;
	for (j = 0; j < len; j++)
	{
		buf[j+1] = data[j];
	}

	// send the SPI message (all of the above fields, inc. buffers)
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);

	return ret;
}

int cc1200_read_rxfifo(uint8_t *data, uint8_t len)
{
	int ret;
	struct spi_ioc_transfer transfer = {
		.tx_buf = (unsigned long)buf,
		.rx_buf = (unsigned long)buf,
		.len = 0,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};
	
	buf[transfer.len++] = CC1200_FIFO | CC1200_READ_BIT | CC1200_BURST_BIT;
	transfer.len += len;

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
	if(ret < 0)
		return ret;
	else
	{
		int j;
		for (j = 0; j < transfer.len; j++)
		{
			data[j] = buf[j+1];
		}
	}

	// send the SPI message (all of the above fields, inc. buffers)
	return ret;
}

int cc1200_init(char * spi_path)
{
	uint8_t partnum = 0;
	uint8_t partver = 0;

	// The following calls set up the CC1200 SPI bus properties
	if((fd = open(spi_path, O_RDWR))<0){
		perror("SPI Error: Can't open device.");
		return -1;
	}
	if(ioctl(fd, SPI_IOC_WR_MODE, &mode)==-1){
		perror("SPI: Can't set SPI mode.");
		return -1;
	}
	if(ioctl(fd, SPI_IOC_RD_MODE, &mode)==-1){
		perror("SPI: Can't get SPI mode.");
		return -1;
	}
	if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits)==-1){
		perror("SPI: Can't set bits per word.");
		return -1;
	}
	if(ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits)==-1){
		perror("SPI: Can't get bits per word.");
		return -1;
	}
	if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed)==-1){
		perror("SPI: Can't set max speed HZ");
		return -1;
	}
	if(ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed)==-1){
		perror("SPI: Can't get max speed HZ.");
		return -1;
	}

	// Check that the properties have been set
	printf("SPI Mode is: %d\n", mode);
	printf("SPI Bits is: %d\n", bits);
	printf("SPI Speed is: %d\n", speed);

	// Reset Radio
	cc1200_cmd_strobe(CC1200_SRES);

	// Get Chip Info
	cc1200_read_register(CC1200_PARTNUMBER, &partnum);
	cc1200_read_register(CC1200_PARTVERSION, &partver);
	printf("CC1200 Chip Number: 0x%x Chip Version: 0x%x\n", partnum, partver);

	return 0;
}
