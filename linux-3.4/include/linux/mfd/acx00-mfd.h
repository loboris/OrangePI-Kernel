/*
 * include/linux/mfd/acx00/core.h -- Core interface for ACX00
 *
 * Copyright 2009 Wolfson Microelectronics PLC.
 *
 * Author: Mark Brown <broonie@opensource.wolfsonmicro.com>
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#ifndef __MFD_ACX00_CORE_H__
#define __MFD_ACX00_CORE_H__

#include <linux/mutex.h>
#include <linux/interrupt.h>

/* GPIOs in the chip are numbered from 1-11 */
#define ACX00_IRQ_GPIO(x) (x + ACX00_IRQ_TEMP_WARN)

struct acx00 {
	struct mutex lock;

	struct device *dev;
	struct regmap *regmap;
	int irq;
};

/* Device I/O API */
int acx00_reg_read(struct acx00 *acx00, unsigned short reg);
int acx00_reg_write(struct acx00 *acx00, unsigned short reg, unsigned short val);

int acx00_set_bits(struct acx00 *acx00, unsigned short reg,
		    unsigned short mask, unsigned short val);

#endif
