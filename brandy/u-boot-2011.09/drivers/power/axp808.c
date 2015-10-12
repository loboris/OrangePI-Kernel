/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Liao Yongming <liaoyongming@allwinnertech.com>
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
#include <common.h>
#include <power/axp808_reg.h>
#include <pmu.h>
#include "axp.h"

extern int axp808_set_supply_status(int vol_name, int vol_value, int onoff);
extern int axp808_set_supply_status_byname(char *vol_name, int vol_value, int onoff);
extern int axp808_probe_supply_status(int vol_name, int vol_value, int onoff);
extern int axp808_probe_supply_status_byname(char *vol_name);
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_probe(void)
{
	u8    pmu_type;

    axp_i2c_config(SUNXI_AXP_808, AXP808_ADDR);

	/*the different between AXP808 and AXp806*/
    //axp_i2c_write(AXP808_ADDR, 0xff, 0x10);
	if(axp_i2c_read(AXP808_ADDR, BOOT_POWER808_VERSION, &pmu_type))
	{
		printf("axp read error\n");

		return -1;
	}

    pmu_type &= 0xCF;
	if(pmu_type == 0x40)
	{
        u8 data = 0;
		/* pmu type AXP808 */
		tick_printf("PMU: AXP808\n");
		if(axp_i2c_read(AXP808_ADDR,BOOT_POWER808_DCFREQ_SET,&data))
		{
			printf("axp 808 read DCFREQ error\n");
			return -1;
		}
		data &= (~(0x3 << 4));
		data |= (0x2 << 4);

		axp_i2c_write(AXP808_ADDR,BOOT_POWER808_DCFREQ_SET,data);
		debug("PMU : data is %x\n",data);
		return 0;
	}

	return -1;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_set_coulombmeter_onoff(int onoff)
{
    return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_set_charge_control(void)
{
    return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_probe_battery_ratio(void)
{
	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_probe_power_status(void)
{
	return 0;
}

/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_probe_battery_exist(void)
{
    return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_probe_battery_vol(void)
{
    return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_probe_key(void)
{
    return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_probe_pre_sys_mode(void)
{
    return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_set_next_sys_mode(int data)
{
	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_probe_this_poweron_cause(void)
{
    return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_set_power_off(void)
{
	u8 reg_value;

	if(axp_i2c_read(AXP808_ADDR, BOOT_POWER808_OFF_CTL, &reg_value))
    {
        return -1;
    }
    reg_value |= 1 << 7;
	if(axp_i2c_write(AXP808_ADDR, BOOT_POWER808_OFF_CTL, reg_value))
    {
        return -1;
    }

    return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_set_power_onoff_vol(int set_vol, int stage)
{
	u8 reg_value;

	if(!set_vol)
	{
		if(!stage)
		{
			set_vol = 3300;
		}
		else
		{
			set_vol = 2900;
		}
	}
	if(axp_i2c_read(AXP808_ADDR, BOOT_POWER808_VOFF_SETTING, &reg_value))
    {
        return -1;
    }
	reg_value &= 0xf8;
	if(set_vol >= 2600 && set_vol <= 3300)
	{
		reg_value |= (set_vol - 2600)/100;
	}
	else if(set_vol <= 2600)
	{
		reg_value |= 0x00;
	}
	else
	{
		reg_value |= 0x07;
	}
	if(axp_i2c_write(AXP808_ADDR, BOOT_POWER808_VOFF_SETTING, reg_value))
    {
        return -1;
    }

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_set_charge_current(int current)
{
	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_probe_charge_current(void)
{
	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_set_vbus_cur_limit(int current)
{
    return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_probe_vbus_cur_limit(void)
{
    return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_set_vbus_vol_limit(int vol)
{
    return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_probe_int_pending(uchar *addr)
{
	int   i;

	for(i=0;i<2;i++)
	{
	    if(axp_i2c_read(AXP808_ADDR, BOOT_POWER808_INTSTS1 + i, addr + i))
	    {
	        return -1;
	    }
	}

	for(i=0;i<2;i++)
	{
	    if(axp_i2c_write(AXP808_ADDR, BOOT_POWER808_INTSTS1 + i, 0xff))
	    {
	        return -1;
	    }
	}

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_probe_int_enable(uchar *addr)
{
	int   i;
	uchar  int_reg = BOOT_POWER808_INTEN1;

	for(i=0;i<2;i++)
	{
		if(axp_i2c_read(AXP808_ADDR, int_reg, addr + i))
	    {
	        return -1;
	    }
	    int_reg ++;
	}

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：
*
*
************************************************************************************************************
*/
int axp808_set_int_enable(uchar *addr)
{
	int   i;
	uchar  int_reg = BOOT_POWER808_INTEN1;

	for(i=0;i<2;i++)
	{
		if(axp_i2c_write(AXP808_ADDR, int_reg, addr[i]))
	    {
	        return -1;
	    }
	    int_reg ++;
	}

	return 0;
}


sunxi_axp_module_init("axp808", SUNXI_AXP_808);


