/*
 * (C) Copyright -
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <liaoyongming@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version  of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.,  Temple Place, Suite , Boston,
 * MA - USA
*/
#define __OZ80120_C__

#include <common.h>
#include <sys_config.h>

static int gpio_value[4][3] = {
    {0, 0, 0}, //1.2v
    {0, 0, 1}, //1.1v
    {0, 1, 0}, //1v
    {1, 0, 0}, //0.9v
};

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
static u8 power_oz_probe_gpio(int set_vol)
{
	u8 index = 0;
	if(set_vol <= 900)
	{
		set_vol = 900;
		index = 3;
	}
	else if(set_vol <= 1000)
	{
		set_vol = 1000;
		index = 2;
	}
	else if(set_vol <= 1100)
	{
		set_vol = 1100;
		index = 1;
	}
	else if((set_vol <= 1200) || (set_vol > 1200))
	{
		set_vol = 1200;
		index = 0;
	}
	return index;
}

/*
************************************************************************************************************
*
*                                             function
*
*    函数名称：power_oz_patch_gpio
*
*    参数列表：
*
*    返回值  ：
*
*    说明    ：change the gpio data status in sys_config.fex with the vol input 
*
*
************************************************************************************************************
*/
static int power_oz_patch_gpio(int set_vol)
{
	int ret;
	int i;
	u8 index;
	char sub_name[16];
	user_gpio_set_t oz_gpio_cfg[3];
	memset(oz_gpio_cfg, 0, sizeof(oz_gpio_cfg));

	ret = script_parser_fetch("external_power", "a15_vset1", (int*)&oz_gpio_cfg[0], sizeof(oz_gpio_cfg[0])/sizeof(int));
	if(ret)
	{
		printf("get oz power cfg failed\n");
		return -1;
	}

	ret = script_parser_fetch("external_power", "a15_vset2", (int*)&oz_gpio_cfg[1], sizeof(oz_gpio_cfg[1])/sizeof(int));
	if(ret)
	{
		printf("get oz power cfg failed\n");
		return -1;
	}

	ret = script_parser_fetch("external_power", "a15_vset3", (int*)&oz_gpio_cfg[2], sizeof(oz_gpio_cfg[2])/sizeof(int));
	if(ret)
	{
		printf("get oz power cfg failed\n");
		return -1;
	}

	//get the gpio status with the set_vol
	index = power_oz_probe_gpio(set_vol);

	//配置gpio data
	for(i = 0; i < 3; i++)
	{
		oz_gpio_cfg[i].data = gpio_value[index][i];
		memset(sub_name, 0, 16);
		sprintf(sub_name, "%s%d", "a15_vset", i+1);
		ret = script_parser_patch("external_power", sub_name, (void *)&oz_gpio_cfg[i], sizeof(oz_gpio_cfg[i])/sizeof(int));
		if(ret)
		{
			printf("patch gpio config failed\n");
			return -2;
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
int power_oz_probe(void)
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
		printf("patch a15_pwr_en config failed\n");
		return -2;
	}

	oz_power_hd = gpio_request_simple("external_power", "a15_pwr_en");
	if(oz_power_hd)
	{
		printf("set gpio failed\n");
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
*				int set_vol: not using
*				  int onoff: not using	
*    返回值  ：
*
*    说明    ：根据sys_config.fex 配置进行设置电压, 先设置电压，在使能或者关闭
*
*
************************************************************************************************************
*/
int power_oz_set_dcdc(int set_vol, int onoff)
{
	int ret;
	u32 oz_power_hd;
	user_gpio_set_t oz_gpio_cfg[1];

	//配置输出电压
	if(set_vol > 0)
	{
		ret = power_oz_patch_gpio(set_vol);
		if(ret)
		{
			printf("oz power patch config failed\n");
			return -1;
		}

		oz_power_hd = gpio_request_simple("external_power", "a15_vset1");
		if(oz_power_hd)
		{
			printf("set gpio failed\n");
			return -3;
		}

		oz_power_hd = gpio_request_simple("external_power", "a15_vset2");
		if(oz_power_hd)
		{
			printf("set gpio failed\n");
			return -3;
		}

		oz_power_hd = gpio_request_simple("external_power", "a15_vset3");
		if(oz_power_hd)
		{
			printf("set gpio failed\n");
			return -3;
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
		printf("patch a15_pwr_en config failed\n");
		return -2;
	}

	oz_power_hd = gpio_request_simple("external_power", "a15_pwr_en");
	if(oz_power_hd)
	{
		printf("set gpio failed\n");
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
*    说明    ：
*
*
************************************************************************************************************
*/
int power_oz_set_power_output(void)
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
	printf("oz dcdc = %d, onoff=%d\n",  power_vol_d, onoff);
#else
	printf("oz dcdc = %d\n",  power_vol_d);
#endif
	power_oz_set_dcdc(power_vol_d, onoff);
	return 0;
}

