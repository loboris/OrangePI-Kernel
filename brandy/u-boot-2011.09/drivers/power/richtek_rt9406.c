/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <liaoyongming@allwinnertech.com>
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
#define __RICHTEK_RT9406_C__

#include <common.h>
#include <power/richtek_rt9406.h>
#include <i2c.h>
#include <sys_config.h>

#define RICH_TWI_HOST_ID		0	//twi0
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
*    说明    ：读取升压，降压的陡度
*
*
************************************************************************************************************
*/
int power_rich_probe_rate(void)
{
	u8 reg_value = 0;
	if(i2c_read(RICH_TWI_HOST_ID, RICH_ADDR, POWERRICH_RATEOUT_CTL1, 1, &reg_value, 1))
	{
		printf("rich i2c read rich failed\n");
		return -1;
	}

	return reg_value;
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
*    说明    ：设置升压，降压的陡度
*
*
************************************************************************************************************
*/
static int power_rich_set_rate(u8 rate)
{
	u8 reg_value = rate;
	if(i2c_write(RICH_TWI_HOST_ID, RICH_ADDR, POWERRICH_RATEOUT_CTL1, 1, &reg_value, 1))
	{
		printf("rich i2c write rich failed\n");
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
*    说明    ：设置输出电压(范围: 500mv - 3000mv )
*
*
************************************************************************************************************
*/
int power_rich_set_dcdc(int set_vol, int onoff)
{
	u8 data;
	int ret;
	u32 oz_power_hd;
	user_gpio_set_t oz_gpio_cfg[1];

	printf("rich set_vol=%d\n", set_vol);

	if(set_vol > 0)
	{
		if(set_vol <= 500)
		{
			set_vol = 1030;
		}
		else if(set_vol > 3000)
		{
			set_vol = 3000;
		}
		data = ((set_vol - 500) / 10 + 1);

		ret = i2c_write(RICH_TWI_HOST_ID, RICH_ADDR, POWERRICH_DCOUT_VOL, 1, &data, 1);
		if(ret)
		{
			printf("rich i2c write rich failed\n");
			return -2;
		}
	}

	if(onoff < 0)
	{
		return 0;
	}

	memset(oz_gpio_cfg, 0, sizeof(oz_gpio_cfg));
	ret = script_parser_fetch("external_power", "a15_pwr_en", (int*)&oz_gpio_cfg[0], sizeof(oz_gpio_cfg[0])/sizeof(int));
	if(ret)
	{
		printf("get a15_pwr_en cfg failed\n");
		return -1;
	}

	if(onoff == 0)
	{
		oz_gpio_cfg[0].data = 0;
	}
	else
	{
		oz_gpio_cfg[0].data = 1;
	}
	//配置使能gpio
	ret = script_parser_patch("external_power", "a15_pwr_en", (int*)&oz_gpio_cfg[0], sizeof(oz_gpio_cfg[0])/sizeof(int));
	if(ret)
	{
		printf("patch rich_en gpio config failed\n");
		return -2;
	}

	oz_power_hd = gpio_request_simple("external_power", "a15_pwr_en");
	if(oz_power_hd)
	{
		printf("set a15_pwr_en failed\n");
		return -3;
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
*    说明    ：读取输出电压vol
*
*
************************************************************************************************************
*/
int power_rich_probe_dcdc(void)
{
	int vol = 0;
	u8 reg_value = 0;
	if(i2c_read(RICH_TWI_HOST_ID, RICH_ADDR, POWERRICH_RATEOUT_CTL1, 1, &reg_value, 1))
	{
		printf("i2c read rich error\n");
		return -1;
	}

	if(reg_value > 0)
	{
		vol = (reg_value - 1)*10 + 500;
	}

	return vol;
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
int power_rich_probe(void)
{
	int ret;
	u32 oz_power_hd;
	user_gpio_set_t oz_gpio_cfg[1];

	memset(oz_gpio_cfg, 0, sizeof(oz_gpio_cfg));
	ret = script_parser_fetch("external_power", "a15_pwr_en", (int*)&oz_gpio_cfg[0], sizeof(oz_gpio_cfg[0])/sizeof(int));
	if(ret)
	{
		printf("get a15_pwr_en cfg failed\n");
		return -1;
	}

	oz_gpio_cfg[0].data = 1;

	//配置使能gpio
	ret = script_parser_patch("external_power", "a15_pwr_en", (int*)&oz_gpio_cfg[0], sizeof(oz_gpio_cfg[0])/sizeof(int));
	if(ret)
	{
		printf("patch rich_en gpio config failed\n");
		return -2;
	}

	oz_power_hd = gpio_request_simple("external_power", "a15_pwr_en");
	if(oz_power_hd)
	{
		printf("set a15_pwr_en failed\n");
		return -3;
	}

	if(power_rich_set_rate(0x03))
	{
		printf("rich power write error\n");
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
int power_rich_set_power_output(void)
{
	uint ret;
	int onoff;
	int power_vol;
	int  power_vol_d;
	ret = script_parser_fetch_subkey_start("slave_power_sply");
    if(!ret)
    {
        printf("unable to set slave power supply\n");
		return -1;
	}
	ret = script_parser_fetch("slave_power_sply", "dcdc_vol", &power_vol, 1);
	if(ret)
	{
		printf("unable to get dcdc vol\n");
		return -1;
	}
	onoff = -1;
	power_vol_d = 0;
	if(power_vol > 10000)
	{
		onoff = 1;
		power_vol_d = power_vol%10000;
	}
#if defined(CONFIG_SUNXI_AXP_CONFIG_ONOFF)
	else if(power_vol > 0)
	{
		onoff = 0;
		power_vol_d = power_vol;
	}
#endif
	else if(power_vol == 0)
	{
		onoff = 0;
	}
#if defined(CONFIG_SUNXI_AXP_CONFIG_ONOFF)
	printf("rich dcdc = %d, onoff=%d\n",  power_vol_d, onoff);
#else
	printf("rich dcdc = %d\n",  power_vol_d);
#endif
	power_rich_set_dcdc(power_vol_d, onoff);
	return 0;
}

