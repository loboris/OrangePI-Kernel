/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef   __AXP808_REGS_H__
#define   __AXP808_REGS_H__

#define   AXP808_ADDR              (0x10)

//define AXP808 REGISTER
#define   BOOT_POWER808_STARUP_SRC             			(0x00)
#define   BOOT_POWER808_VERSION         	   			(0x03)
#define   BOOT_POWER808_DATA_BUFFER0        			(0x04)
#define   BOOT_POWER808_DATA_BUFFER1        			(0x05)
#define   BOOT_POWER808_DATA_BUFFER2        			(0x06)
#define   BOOT_POWER808_DATA_BUFFER3        			(0x07)

#define   BOOT_POWER808_OUTPUT_CTL1     	   			(0x10)
#define   BOOT_POWER808_OUTPUT_CTL2     	   			(0x11)

#define   BOOT_POWER808_DCAOUT_VOL                  	(0x12)
#define   BOOT_POWER808_DCBOUT_VOL          			(0x13)
#define   BOOT_POWER808_DCCOUT_VOL          			(0x14)
#define   BOOT_POWER808_DCDOUT_VOL          			(0x15)
#define   BOOT_POWER808_DCEOUT_VOL          			(0x16)
#define   BOOT_POWER808_ALDO1OUT_VOL                    (0x17)
#define   BOOT_POWER808_ALDO2OUT_VOL                    (0x18)
#define   BOOT_POWER808_ALDO3OUT_VOL                    (0x19)
#define   BOOT_POWER808_DCMOD_CTL1                  	(0x1A)
#define   BOOT_POWER808_DCMOD_CTL2                  	(0x1B)
#define   BOOT_POWER808_DCFREQ_SET                  	(0x1C)
#define   BOOT_POWER808_DCMONITOR_CTL                  	(0x1D)

#define   BOOT_POWER808_BLDO1OUT_VOL                    (0x20)
#define   BOOT_POWER808_BLDO2OUT_VOL                    (0x21)
#define   BOOT_POWER808_BLDO3OUT_VOL                    (0x22)
#define   BOOT_POWER808_BLDO4OUT_VOL                    (0x23)

#define   BOOT_POWER808_CLDO1OUT_VOL                    (0x24)
#define   BOOT_POWER808_CLDO2OUT_VOL                    (0x25)
#define   BOOT_POWER808_CLDO3OUT_VOL                    (0x26)

#define   BOOT_POWER808_VOFF_SETTING                    (0x31)
#define   BOOT_POWER808_OFF_CTL             			(0x32)
#define   BOOT_POWER808_POK_SETTING                     (0x36)

#define   BOOT_POWER808_INTEN1              			(0x40)
#define   BOOT_POWER808_INTEN2              			(0x41)
#define   BOOT_POWER808_INTSTS1             			(0x48)
#define   BOOT_POWER808_INTSTS2             			(0x49)

#endif /* __AXP808_REGS_H__ */
