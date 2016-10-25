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

#include <stdint.h>
#include <unistd.h>
#include <linux/types.h>
#include "cc1200-rf-cfg.h"

int cc1200_init(char * spi_path);
int cc1200_cmd_strobe(uint8_t cmd);
int cc1200_get_status(uint8_t *status);
int cc1200_write_register(uint16_t reg, uint8_t value);
void cc1200_write_reg_settings(const registerSetting_t *reg_settings,
		uint16_t sizeof_reg_settings);
int cc1200_read_register(uint16_t reg, uint8_t *data);
int cc1200_write_txfifo(uint8_t *data, uint8_t len);
int cc1200_read_rxfifo(uint8_t *data, uint8_t len);
