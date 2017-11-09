/* 
 ***************************************************************************************
 * 
 * config.c
 * 
 * Hawkview ISP - config.c module
 * 
 * Copyright (c) 2014 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 * 
 * Version		  Author         Date		    Description
 * 
 *   2.0		  Yang Feng   	2014/03/11	      Second Version
 * 
 ****************************************************************************************
 */

#include "config.h"
#include "platform_cfg.h"
#include "isp_cfg/isp_cfg.h"
#define SIZE_OF_LSC_TBL_MOD0     7*768*2
#define SIZE_OF_LSC_TBL_MOD1     8*768*2
#define SIZE_OF_HDR_TBL     4*256*2
#define SIZE_OF_GAMMA_TBL   256*2

void set_used(struct sensor_config_init *sensor_cfg, void *value, int len)
{
	sensor_cfg->used = *(int *)value;
	vfe_dbg(0,"sensor_cfg->used = %d!\n", sensor_cfg->used);
}
void set_csi_sel(struct sensor_config_init *sensor_cfg, void *value, int len)
{
	sensor_cfg->csi_sel 		 = *(int *)value;
}
void set_device_sel(struct sensor_config_init *sensor_cfg, void *value, int len)
{
	sensor_cfg->device_sel 		 = *(int *)value;
}
void set_sensor_twi_id(struct sensor_config_init *sensor_cfg, void *value, int len)
{
	sensor_cfg->twi_id 			 = *(int *)value;
}
void set_power_settings_enable(struct sensor_config_init *sensor_cfg, void *value, int len)
{
	sensor_cfg->power_settings_enable = *(int *)value;
}

void set_iovdd(struct sensor_config_init *sensor_cfg, void *value, int len)
{
	strcpy(sensor_cfg->sub_power_str[ENUM_IOVDD],(char *)value);
	vfe_dbg(0,"sub_power_str[ENUM_IOVDD] = %s!\n", sensor_cfg->sub_power_str[ENUM_IOVDD]);
}
void set_iovdd_vol(struct sensor_config_init *sensor_cfg, void *value, int len)
{
	sensor_cfg->sub_power_vol[ENUM_IOVDD] 		 = *(int *)value;
}
void set_avdd(struct sensor_config_init *sensor_cfg, void *value, int len)
{
	strcpy(sensor_cfg->sub_power_str[ENUM_AVDD],(char *)value);
}
void set_avdd_vol(struct sensor_config_init *sensor_cfg, void *value, int len)
{
	sensor_cfg->sub_power_vol[ENUM_AVDD] 		 = *(int *)value;
}
void set_dvdd(struct sensor_config_init *sensor_cfg, void *value, int len)
{
	strcpy(sensor_cfg->sub_power_str[ENUM_DVDD],(char *)value);
}
void set_dvdd_vol(struct sensor_config_init *sensor_cfg, void *value, int len)
{
	sensor_cfg->sub_power_vol[ENUM_DVDD] 		 = *(int *)value;
}
void set_afvdd(struct sensor_config_init *sensor_cfg, void *value, int len)
{
	strcpy(sensor_cfg->sub_power_str[ENUM_AFVDD],(char *)value);
}
void set_afvdd_vol(struct sensor_config_init *sensor_cfg, void *value, int len)
{
	sensor_cfg->sub_power_vol[ENUM_AFVDD] 		 = *(int *)value;
}
void set_detect_sensor_num(struct sensor_config_init *sensor_cfg, void *value, int len)
{
	sensor_cfg->detect_sensor_num 	 = *(int *)value;
}

void set_sensor_name(struct sensor_config_init *sensor_cfg, void *value, int sel)
{
	strcpy(sensor_cfg->camera_inst[sel].name, (char *)value);
}
void set_sensor_twi_addr(struct sensor_config_init *sensor_cfg, void *value, int sel)
{
	sensor_cfg->camera_inst[sel].i2c_addr  		= *(int *)value;
}
void set_sensor_type(struct sensor_config_init *sensor_cfg, void *value, int sel)
{
	sensor_cfg->camera_inst[sel].sensor_type 	= *(int *)value;
}
void set_sensor_stby_mode(struct sensor_config_init *sensor_cfg, void *value, int sel)
{
	sensor_cfg->camera_inst[sel].stdby_mode  	= *(int *)value;
}
void set_sensor_hflip(struct sensor_config_init *sensor_cfg, void *value, int sel)
{
	sensor_cfg->camera_inst[sel].vflip  		= *(int *)value;
}
void set_sensor_vflip(struct sensor_config_init *sensor_cfg, void *value, int sel)
{
	sensor_cfg->camera_inst[sel].hflip  		= *(int *)value;
}
void set_act_name(struct sensor_config_init *sensor_cfg, void *value, int sel)
{
	strcpy(sensor_cfg->camera_inst[sel].act_name, (char *)value);
}
void set_act_twi_addr(struct sensor_config_init *sensor_cfg, void *value, int sel)
{
	sensor_cfg->camera_inst[sel].act_i2c_addr  	= *(int *)value;
}
void set_isp_cfg_name(struct sensor_config_init *sensor_cfg, void *value, int sel)
{
	strcpy(sensor_cfg->camera_inst[sel].isp_cfg_name, (char *)value);
}

enum ini_item_type {
	INTEGER,
	STRING,
};

struct SensorParamAttribute
{
	char *sub;
	int len;
	enum ini_item_type type;
	void (*set_param)(struct sensor_config_init *, void *, int len);
};

static struct SensorParamAttribute SensorParamCommon[] =
{
	{	"used"        			, 1 , INTEGER	,	 set_used        	 	 ,},
	{	"csi_sel"     			, 1 , INTEGER	,	 set_csi_sel     	 	 ,},
	{	"device_sel"    		, 1 , INTEGER	,	 set_device_sel        ,},
	{	"sensor_twi_id"     	, 1 , INTEGER	,	 set_sensor_twi_id     ,},
	{	"power_settings_enable"     	, 1 , INTEGER	,	 set_power_settings_enable     ,},
	{	"iovdd"					, 1 , STRING	,	 set_iovdd			  ,},
	{	"iovdd_vol"  			, 1 , INTEGER	,	 set_iovdd_vol  	      ,},
	{	"avdd"        			, 1 , STRING	,	 set_avdd        	  	,},
	{	"avdd_vol"				, 1 , INTEGER	,	 set_avdd_vol		 	 ,},
	{	"dvdd"           		, 1 , STRING	,	 set_dvdd              ,},
	{	"dvdd_vol"				, 1 , INTEGER	,	 set_dvdd_vol		  	,},
	{	"afvdd" 				, 1 , STRING	,	 set_afvdd 		      ,},
	{	"afvdd_vol"     		, 1 , INTEGER	,	 set_afvdd_vol         ,},
	{	"detect_sensor_num"		, 1 , INTEGER	,	 set_detect_sensor_num	 ,},
};
static struct SensorParamAttribute SensorParamDetect[] =
{
	{	"sensor_name"		, 1 ,	STRING,   set_sensor_name		    ,},
	{	"sensor_twi_addr"	, 1 ,	INTEGER,   set_sensor_twi_addr	    ,},
	{	"sensor_type"		, 1 ,	INTEGER,   set_sensor_type		    ,},
	{	"sensor_stby_mode"	, 1 ,	INTEGER,   set_sensor_stby_mode		,},
	{	"sensor_hflip"		, 1 ,	INTEGER,   set_sensor_hflip			,},
	{	"sensor_vflip"		, 1 ,	INTEGER,   set_sensor_vflip			,},
	{	"act_name"			, 1 ,	STRING,   set_act_name				,},
	{	"act_twi_addr"		, 1 ,	INTEGER,   set_act_twi_addr			,},
	{	"isp_cfg_name"		, 1 ,	STRING,   set_isp_cfg_name		    ,},
};
int fetch_sensor_list(struct sensor_config_init *sensor_cfg_ini , char *main, struct cfg_section *cfg_section)
{
	int i, j;
	struct cfg_subkey subkey;
	struct SensorParamAttribute *SensorCommon;
	static struct SensorParamAttribute *SensorDetect;
	char sub_name[128] = {0};
	SensorCommon = &SensorParamCommon[0];
	//fetch sensor common config;
	vfe_print("fetch sensor common config! \n");
	for (i = 0; i < ARRAY_SIZE(SensorParamCommon);  i++)
	{
		if(main == NULL || SensorCommon->sub == NULL)
		{
			vfe_warn("fetch_sensor_list main or SensorCommon->sub is NULL!\n");
			continue;
		}
		if(SensorCommon->type == INTEGER)
		{
			if (CFG_ITEM_VALUE_TYPE_INT != cfg_get_one_subkey(cfg_section, main, SensorCommon->sub, &subkey))
			{
				vfe_dbg(0,"Warning: %s->%s,apply default value!\n", main, SensorCommon->sub);
			}
			else
			{
				if(SensorCommon->set_param)
				{
					SensorCommon->set_param(sensor_cfg_ini, (void *)&subkey.value.val, SensorCommon->len);
					vfe_dbg(0,"fetch sensor cfg ini: %s->%s  = %d\n",main, SensorCommon->sub,subkey.value.val);
				}
			}
		}
		else if(SensorCommon->type == STRING)
		{
			if (CFG_ITEM_VALUE_TYPE_STR != cfg_get_one_subkey(cfg_section, main, SensorCommon->sub, &subkey))
			{
				vfe_dbg(0,"Warning: %s->%s,apply default value!\n", main, SensorCommon->sub);
			}
			else
			{
				if(SensorCommon->set_param)
				{
					if(!strcmp(subkey.value.str, "\"\""))
					{
						strcpy(subkey.value.str,"");
					}
					SensorCommon->set_param(sensor_cfg_ini, (void *)subkey.value.str, SensorCommon->len);
					vfe_dbg(0,"fetch sensor cfg ini: %s->%s  = %s\n",main, SensorCommon->sub,subkey.value.str);
				}
			}
		}
		SensorCommon ++;
	}
	//fetch sensor detect config;
	vfe_print("fetch sensor detect config! \n");
	if(sensor_cfg_ini->detect_sensor_num > MAX_SENSOR_DETECT_NUM)
	{
		vfe_err("sensor_num = %d > MAX_SENSOR_DETECT_NUM = %d\n", sensor_cfg_ini->detect_sensor_num,MAX_SENSOR_DETECT_NUM);
		sensor_cfg_ini->detect_sensor_num = 1;
	}
	for (j = 0; j < sensor_cfg_ini->detect_sensor_num;  j++)
	{
		SensorDetect = &SensorParamDetect[0];
		for (i = 0; i < ARRAY_SIZE(SensorParamDetect);  i++)
		{
			if(main == NULL || SensorDetect->sub == NULL)
			{
				vfe_warn("fetch_sensor_list main or SensorDetect->sub is NULL!\n");
				continue;
			}
			sprintf(sub_name, "%s%d",SensorDetect->sub, j);
			if(SensorDetect->type == INTEGER)
			{
				if (CFG_ITEM_VALUE_TYPE_INT != cfg_get_one_subkey(cfg_section, main, sub_name, &subkey))
				{
					vfe_dbg(0,"Warning: %s->%s,apply default value!\n", main, SensorDetect->sub);
				}
				else
				{
					if(SensorDetect->set_param)
					{
						SensorDetect->set_param(sensor_cfg_ini, (void *)&subkey.value.val, j);
						vfe_dbg(0,"fetch sensor cfg ini: %s->%s  = %d\n",main, sub_name,subkey.value.val);
					}
				}
			}
			else if(SensorDetect->type == STRING)
			{
				if (CFG_ITEM_VALUE_TYPE_STR != cfg_get_one_subkey(cfg_section, main, sub_name, &subkey))
				{
					vfe_dbg(0,"Warning: %s->%s,apply default value!\n", main, sub_name);
				}
				else
				{
					if(SensorDetect->set_param)
					{
						if(!strcmp(subkey.value.str, "\"\""))
						{
							strcpy(subkey.value.str,"");
						}
						SensorDetect->set_param(sensor_cfg_ini, (void *)subkey.value.str, j);
						vfe_dbg(0,"fetch sensor cfg ini: %s->%s  = %s\n",main, sub_name,subkey.value.str);
					}
				}
			}
			SensorDetect ++;
		}
	}
	vfe_dbg(0,"fetch sensor_list done!\n");
	return 0;
}
int parse_sensor_list_info(struct sensor_config_init *sensor_cfg_ini , char *pos)
{
	int ret = 0;
	struct cfg_section *cfg_section;
	char sensor_list_cfg[128];
	vfe_print("fetch %s sensor list info start!\n",pos);
	sprintf(sensor_list_cfg, "/system/etc/hawkview/sensor_list_cfg.ini");
	if(strcmp(pos, "rear") && strcmp(pos, "REAR")  && strcmp(pos, "FRONT")  && strcmp(pos, "front") )
	{
		vfe_err("Camera position config ERR! POS = %s, please check the key <vip_dev(x)_pos> in sys_config!\n", pos);
	}
	vfe_print("Fetch sensor list form\"%s\"\n",sensor_list_cfg);
	cfg_section_init(&cfg_section);
	ret = cfg_read_ini(sensor_list_cfg, &cfg_section);
	if(ret == -1)
	{
		cfg_section_release(&cfg_section);
		goto parse_sensor_list_info_end;
	}
	if(strcmp(pos, "rear") == 0 || strcmp(pos, "REAR") == 0)
	{
		fetch_sensor_list(sensor_cfg_ini, "rear_camera_cfg", cfg_section);
	}
	else
	{
		fetch_sensor_list(sensor_cfg_ini, "front_camera_cfg", cfg_section);
	}
	cfg_section_release(&cfg_section);
parse_sensor_list_info_end:
	vfe_print("fetch %s sensor list info end!\n", pos);
	return ret;
}
int fetch_config(struct vfe_dev *dev)
{
#ifdef VFE_SYS_CONFIG
  int ret,i;
  char vfe_para[16] = {0};
  char dev_para[32] = {0};

  script_item_u   val;
  script_item_value_type_e	type;

  sprintf(vfe_para, "csi%d", dev->id);
  /* fetch device quatity issue */
  type = script_get_item(vfe_para,"vip_dev_qty", &val);
  if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
	dev->dev_qty=1;
    vfe_err("fetch csi_dev_qty from sys_config failed\n");
  } else {
	  dev->dev_qty=val.val;
	  vfe_dbg(0,"vip%d vip_dev_qty=%d\n",dev->id, dev->dev_qty);
  }
	if(dev->vip_define_sensor_list == 0xff)
	{
		type = script_get_item(vfe_para,"vip_define_sensor_list", &val);
		if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
			dev->vip_define_sensor_list = 0;
			vfe_warn("fetch vip_define_sensor_list from sys_config failed\n");
		} else {
			dev->vip_define_sensor_list=val.val;
			vfe_dbg(0,"vip%d vip_define_sensor_list=%d\n",dev->id, dev->vip_define_sensor_list);
		}
	}
	type = script_get_item(vfe_para, "vip_csi_mck", &val);
	for(i=0; i<dev->dev_qty; i++)
	{
		if (SCIRPT_ITEM_VALUE_TYPE_PIO != type) {
			dev->ccm_cfg[i]->gpio.mclk.gpio = GPIO_INDEX_INVALID;
			vfe_dbg(0,"fetch vip_dev%d_power_en from sys_config failed\n", i);
		} else {
			dev->ccm_cfg[i]->gpio.mclk.gpio = val.gpio.gpio;
			dev->ccm_cfg[i]->gpio.mclk.mul_sel = val.gpio.mul_sel;
			dev->ccm_cfg[i]->gpio.mclk.pull = val.gpio.pull;
			dev->ccm_cfg[i]->gpio.mclk.drv_level = val.gpio.drv_level;
			dev->ccm_cfg[i]->gpio.mclk.data = val.gpio.data;
			//printk("mclk:%d,%d,%d,%d,%d\n",i, dev->ccm_cfg[i]->gpio.mclk.mul_sel, dev->ccm_cfg[i]->gpio.mclk.pull, dev->ccm_cfg[i]->gpio.mclk.drv_level,dev->ccm_cfg[i]->gpio.mclk.data);
		}
	}

  for(i=0; i<dev->dev_qty; i++)
  {
    /* i2c and module name*/
    sprintf(dev_para, "vip_dev%d_twi_id", i);
    type = script_get_item(vfe_para, dev_para, &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
      vfe_err("fetch vip_dev%d_twi_id from sys_config failed\n", i);
    } else {
      dev->ccm_cfg[i]->twi_id = val.val;
    }
    dev->ccm_cfg[i]->sensor_cfg_ini = kmalloc(sizeof(struct sensor_config_init),GFP_KERNEL);
    if(!dev->ccm_cfg[i]->sensor_cfg_ini)
    {
    	vfe_err("Sensor cfg ini kmalloc failed!\n");
    }
    memset(dev->ccm_cfg[i]->sensor_cfg_ini, 0,sizeof(struct sensor_config_init));

    if(dev->vip_define_sensor_list == 1)
    {
    	    sprintf(dev_para, "vip_dev%d_pos", i);
	    type = script_get_item(vfe_para, dev_para, &val);
	    if (SCIRPT_ITEM_VALUE_TYPE_STR != type)
	    {
	    	char tmp_str[]="rear";
	    	strcpy(dev->ccm_cfg[i]->sensor_pos, tmp_str);
	    	vfe_err("fetch vip_dev%d_pos from sys_config failed\n", i);
	    } else {
	      strcpy(dev->ccm_cfg[i]->sensor_pos, val.str);
	    }
	    parse_sensor_list_info(dev->ccm_cfg[i]->sensor_cfg_ini, dev->ccm_cfg[i]->sensor_pos);
    }
    ret = strcmp(dev->ccm_cfg[i]->ccm,"");
    if((dev->ccm_cfg[i]->i2c_addr == 0xff) && (ret == 0)) //when insmod without parm
    {
      sprintf(dev_para, "vip_dev%d_twi_addr", i);
      type = script_get_item(vfe_para, dev_para, &val);
      if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        vfe_err("fetch vip_dev%d_twi_addr from sys_config failed\n", i);
      } else {
        dev->ccm_cfg[i]->i2c_addr = val.val;
      }

      sprintf(dev_para, "vip_dev%d_mname", i);
      type = script_get_item(vfe_para, dev_para, &val);
      if (SCIRPT_ITEM_VALUE_TYPE_STR != type) {
        char tmp_str[]="ov5650";
        strcpy(dev->ccm_cfg[i]->ccm,tmp_str);
        vfe_err("fetch vip_dev%d_mname from sys_config failed\n", i);
      } else {
        strcpy(dev->ccm_cfg[i]->ccm,val.str);
        strcpy(dev->ccm_cfg[i]->isp_cfg_name,val.str);

      }
    }

    /* isp used mode */
    sprintf(dev_para, "vip_dev%d_isp_used", i);
    type = script_get_item(vfe_para, dev_para, &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type)
    {
      vfe_dbg(0,"fetch vip_dev%d_isp_used from sys_config failed\n", i);
    } else {
      dev->ccm_cfg[i]->is_isp_used = val.val;
    }

    /* fmt */
    sprintf(dev_para, "vip_dev%d_fmt", i);
    type = script_get_item(vfe_para, dev_para, &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
      vfe_dbg(0,"fetch vip_dev%d_fmt from sys_config failed\n", i);
    } else {
      dev->ccm_cfg[i]->is_bayer_raw = val.val;
    }

    /* standby mode */
    sprintf(dev_para, "vip_dev%d_stby_mode", i);
    type = script_get_item(vfe_para, dev_para, &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
      vfe_dbg(0,"fetch vip_dev%d_stby_mode from sys_config failed\n", i);
    } else {
#ifdef CONFIG_ARCH_SUN8IW8P1
	dev->ccm_cfg[i]->power.stby_mode = POWER_OFF;
#else
      dev->ccm_cfg[i]->power.stby_mode = val.val;
#endif
    }

    /* fetch flip issue */
    sprintf(dev_para, "vip_dev%d_vflip", i);
    type = script_get_item(vfe_para, dev_para, &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
      vfe_dbg(0,"fetch vip_dev%d_vflip from sys_config failed\n", i);
    } else {
      dev->ccm_cfg[i]->vflip = val.val;
    }

    sprintf(dev_para, "vip_dev%d_hflip", i);
    type = script_get_item(vfe_para, dev_para, &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
      vfe_dbg(0,"fetch vip_dev%d_hflip from sys_config failed\n", i);
    } else {
      dev->ccm_cfg[i]->hflip = val.val;
    }

    /* fetch power issue*/
    sprintf(dev_para, "vip_dev%d_iovdd", i);
    type = script_get_item(vfe_para, dev_para, &val);

    if (SCIRPT_ITEM_VALUE_TYPE_STR != type) {
      char null_str[]="";
      strcpy(dev->ccm_cfg[i]->iovdd_str,null_str);
      vfe_dbg(0,"fetch vip_dev%d_iovdd from sys_config failed\n", i);
    } else {
      strcpy(dev->ccm_cfg[i]->iovdd_str,val.str);
    }

    sprintf(dev_para, "vip_dev%d_iovdd_vol", i);
    type = script_get_item(vfe_para,dev_para, &val);
	if (SCIRPT_ITEM_VALUE_TYPE_INT != type)
	{
		dev->ccm_cfg[i]->power.iovdd_vol=0;
		vfe_dbg(0,"fetch vip_dev%d_iovdd_vol from sys_config failed, default =0\n",i);
	}
	else
	 {
		dev->ccm_cfg[i]->power.iovdd_vol=val.val;
	}

    sprintf(dev_para, "vip_dev%d_avdd", i);
    type = script_get_item(vfe_para, dev_para, &val);
    if (SCIRPT_ITEM_VALUE_TYPE_STR != type) {
      char null_str[]="";
      strcpy(dev->ccm_cfg[i]->avdd_str,null_str);
      vfe_dbg(0,"fetch vip_dev%d_avdd from sys_config failed\n", i);
    } else {
      strcpy(dev->ccm_cfg[i]->avdd_str,val.str);
    }

    sprintf(dev_para, "vip_dev%d_avdd_vol", i);
    type = script_get_item(vfe_para,dev_para, &val);
		if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
	    dev->ccm_cfg[i]->power.avdd_vol=0;
			vfe_dbg(0,"fetch vip_dev%d_avdd_vol from sys_config failed, default =0\n",i);
		} else {
	    dev->ccm_cfg[i]->power.avdd_vol=val.val;
	  }

    sprintf(dev_para, "vip_dev%d_dvdd", i);
    type = script_get_item(vfe_para, dev_para, &val);
    if (SCIRPT_ITEM_VALUE_TYPE_STR != type){
      char null_str[]="";
      strcpy(dev->ccm_cfg[i]->dvdd_str,null_str);
      vfe_dbg(0,"fetch vip_dev%d_dvdd from sys_config failed\n", i);
    } else {
      strcpy(dev->ccm_cfg[i]->dvdd_str, val.str);
    }

		sprintf(dev_para, "vip_dev%d_dvdd_vol", i);
    type = script_get_item(vfe_para,dev_para, &val);
		if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
	    dev->ccm_cfg[i]->power.dvdd_vol=0;
			vfe_dbg(0,"fetch vip_dev%d_dvdd_vol from sys_config failed, default =0\n",i);
		} else {
	    dev->ccm_cfg[i]->power.dvdd_vol=val.val;
	  }

    sprintf(dev_para, "vip_dev%d_afvdd", i);
    type = script_get_item(vfe_para, dev_para, &val);
    if (SCIRPT_ITEM_VALUE_TYPE_STR != type) {
      char null_str[]="";
      strcpy(dev->ccm_cfg[i]->afvdd_str,null_str);
      vfe_dbg(0,"fetch vip_dev%d_afvdd from sys_config failed\n", i);
    } else {
      strcpy(dev->ccm_cfg[i]->afvdd_str, val.str);
    }

	  sprintf(dev_para, "vip_dev%d_afvdd_vol", i);
    type = script_get_item(vfe_para,dev_para, &val);
		if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
	    dev->ccm_cfg[i]->power.afvdd_vol=0;
			vfe_dbg(0,"fetch vip_dev%d_afvdd_vol from sys_config failed, default =0\n",i);
		} else {
	    dev->ccm_cfg[i]->power.afvdd_vol=val.val;
	  }

    /* fetch reset/power/standby/flash/af io issue */
    sprintf(dev_para, "vip_dev%d_reset", i);
    type = script_get_item(vfe_para, dev_para, &val);
    if (SCIRPT_ITEM_VALUE_TYPE_PIO != type) {
      dev->ccm_cfg[i]->gpio.reset.gpio = GPIO_INDEX_INVALID;
      vfe_dbg(0,"fetch vip_dev%d_reset from sys_config failed\n", i);
    } else {
      dev->ccm_cfg[i]->gpio.reset.gpio = val.gpio.gpio;
      dev->ccm_cfg[i]->gpio.reset.mul_sel=val.gpio.mul_sel;
			dev->ccm_cfg[i]->gpio.reset.pull = val.gpio.pull;
			dev->ccm_cfg[i]->gpio.reset.drv_level = val.gpio.drv_level;
			dev->ccm_cfg[i]->gpio.reset.data = val.gpio.data;
    }

    sprintf(dev_para, "vip_dev%d_pwdn", i);
    type = script_get_item(vfe_para, dev_para, &val);
    if (SCIRPT_ITEM_VALUE_TYPE_PIO != type){
      dev->ccm_cfg[i]->gpio.pwdn.gpio = GPIO_INDEX_INVALID;
      vfe_dbg(0,"fetch vip_dev%d_stby from sys_config failed\n", i);
    } else {
      dev->ccm_cfg[i]->gpio.pwdn.gpio = val.gpio.gpio;
      dev->ccm_cfg[i]->gpio.pwdn.mul_sel = val.gpio.mul_sel;
			dev->ccm_cfg[i]->gpio.pwdn.pull = val.gpio.pull;
			dev->ccm_cfg[i]->gpio.pwdn.drv_level = val.gpio.drv_level;
			dev->ccm_cfg[i]->gpio.pwdn.data = val.gpio.data;
    }
    sprintf(dev_para, "vip_dev%d_power_en", i);
    type = script_get_item(vfe_para, dev_para, &val);
    if (SCIRPT_ITEM_VALUE_TYPE_PIO != type) {
      dev->ccm_cfg[i]->gpio.power_en.gpio = GPIO_INDEX_INVALID;
      vfe_dbg(0,"fetch vip_dev%d_power_en from sys_config failed\n", i);
    } else {
      dev->ccm_cfg[i]->gpio.power_en.gpio = val.gpio.gpio;
      dev->ccm_cfg[i]->gpio.power_en.mul_sel = val.gpio.mul_sel;
			dev->ccm_cfg[i]->gpio.power_en.pull = val.gpio.pull;
			dev->ccm_cfg[i]->gpio.power_en.drv_level = val.gpio.drv_level;
			dev->ccm_cfg[i]->gpio.power_en.data = val.gpio.data;
    }
    sprintf(dev_para, "vip_dev%d_flash_en", i);
    type = script_get_item(vfe_para, dev_para, &val);
    if (SCIRPT_ITEM_VALUE_TYPE_PIO != type) {
      dev->ccm_cfg[i]->gpio.flash_en.gpio = GPIO_INDEX_INVALID;
      vfe_dbg(0,"fetch vip_dev%d_flash_en from sys_config failed\n", i);
    } else {
      dev->ccm_cfg[i]->gpio.flash_en.gpio = val.gpio.gpio;
      dev->ccm_cfg[i]->gpio.flash_en.mul_sel = val.gpio.mul_sel;
			dev->ccm_cfg[i]->gpio.flash_en.pull = val.gpio.pull;
			dev->ccm_cfg[i]->gpio.flash_en.drv_level = val.gpio.drv_level;
			dev->ccm_cfg[i]->gpio.flash_en.data = val.gpio.data;
			dev->ccm_cfg[i]->flash_used=1;
		sprintf(dev_para, "vip_dev%d_flash_type", i);
		type = script_get_item(vfe_para,dev_para, &val);
		if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
			dev->ccm_cfg[i]->flash_type=0;
			vfe_dbg(0,"fetch vip_dev%d_flash_driver_type from sys_config failed, default =0\n",i);
		} else {
			dev->ccm_cfg[i]->flash_type = val.val;
		}
	
    }

    sprintf(dev_para, "vip_dev%d_flash_mode", i);
    type = script_get_item(vfe_para, dev_para, &val);
    if (SCIRPT_ITEM_VALUE_TYPE_PIO != type) {
      dev->ccm_cfg[i]->gpio.flash_mode.gpio = GPIO_INDEX_INVALID;
      vfe_dbg(0,"fetch vip_dev%d_flash_mode from sys_config failed\n", i);
    } else {
      dev->ccm_cfg[i]->gpio.flash_mode.gpio = val.gpio.gpio;
      dev->ccm_cfg[i]->gpio.flash_mode.mul_sel = val.gpio.mul_sel;
			dev->ccm_cfg[i]->gpio.flash_mode.pull = val.gpio.pull;
			dev->ccm_cfg[i]->gpio.flash_mode.drv_level = val.gpio.drv_level;
			dev->ccm_cfg[i]->gpio.flash_mode.data = val.gpio.data;
    }

    sprintf(dev_para, "vip_dev%d_af_pwdn", i);
    type = script_get_item(vfe_para, dev_para, &val);
    if (SCIRPT_ITEM_VALUE_TYPE_PIO != type) {
      dev->ccm_cfg[i]->gpio.af_pwdn.gpio = GPIO_INDEX_INVALID;
      vfe_dbg(0,"fetch vip_dev%d_af_pwdn from sys_config failed\n", i);
    } else {
      dev->ccm_cfg[i]->gpio.af_pwdn.gpio = val.gpio.gpio;
      dev->ccm_cfg[i]->gpio.af_pwdn.mul_sel = val.gpio.mul_sel;
			dev->ccm_cfg[i]->gpio.af_pwdn.pull = val.gpio.pull;
			dev->ccm_cfg[i]->gpio.af_pwdn.drv_level = val.gpio.drv_level;
			dev->ccm_cfg[i]->gpio.af_pwdn.data = val.gpio.data;
    }

		/* fetch actuator issue */
	  sprintf(dev_para, "vip_dev%d_act_used", i);
	  type = script_get_item(vfe_para, dev_para, &val);
	  if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
		dev->ccm_cfg[i]->act_used= 0;
		vfe_dbg(0,"fetch vip_dev%d_act_used from sys_config failed\n", i);
	  } else {
		dev->ccm_cfg[i]->act_used=val.val;
	  }

    ret = strcmp(dev->ccm_cfg[i]->act_name,"");
	  if((dev->ccm_cfg[i]->act_slave == 0xff) && (ret == 0)) //when insmod without parm
	  {
  	  sprintf(dev_para, "vip_dev%d_act_name", i);
  	  type = script_get_item(vfe_para, dev_para, &val);
  	    if (SCIRPT_ITEM_VALUE_TYPE_STR != type) {
  	      char null_str[]="";
  	      strcpy(dev->ccm_cfg[i]->act_name,null_str);
  	      vfe_dbg(0,"fetch vip_dev%d_act_name from sys_config failed\n", i);
  	    } else {
  	      strcpy(dev->ccm_cfg[i]->act_name,val.str);
  	    }

  		sprintf(dev_para, "vip_dev%d_act_slave", i);
  		type = script_get_item(vfe_para, dev_para, &val);
  		if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
  		  dev->ccm_cfg[i]->act_slave= 0;

  		  vfe_dbg(0,"fetch vip_dev%d_act_slave from sys_config failed\n", i);
  		} else {
  		  dev->ccm_cfg[i]->act_slave=val.val;
  		}
	  }
  //vfe_dbg(0,"act_used=%d, name=%s, slave=0x%x\n",dev->ccm_cfg[0]->act_used,
  //	dev->ccm_cfg[0]->act_name, dev->ccm_cfg[0]->act_slave);
  }
#else
  int type;
#if defined(CONFIG_ARCH_SUN8IW3P1) || defined(CONFIG_ARCH_SUN9IW1P1)
	unsigned int i2c_addr_vip0[2] = {0x78,0xff};
	unsigned char ccm_vip0_dev0[] = {"ov5640",};
  unsigned char ccm_vip0_dev1[] = {"",};
	unsigned int vip0_is_isp_used[2] = {1,1};
  unsigned int vip0_is_bayer_raw[2] = {0,0};
  unsigned int vip0_act_used[2] = {0,0};
  unsigned int vip0_act_addr[2] = {0xff,0xff};
	unsigned char vip0_act_name_dev0[] = {"",};
  unsigned char vip0_act_name_dev1[] = {"",};

	unsigned int i2c_addr_vip1[2] = {0xff,0xff};
  unsigned char ccm_vip1_dev0[] = {"",};
  unsigned char ccm_vip1_dev1[] = {"",};
  unsigned int vip1_is_isp_used[2] = {1,1};
  unsigned int vip1_is_bayer_raw[2] = {0,0};
  unsigned int vip1_act_used[2] = {0,0};
  unsigned int vip1_act_addr[2] = {0xff,0xff};
	unsigned char vip1_act_name_dev0[] = {"",};
  unsigned char vip1_act_name_dev1[] = {"",};
#else
  unsigned int i2c_addr_vip0[2] = {0x6c,0x00};
  unsigned char ccm_vip0_dev0[] = {"ov8825",};
  unsigned char ccm_vip0_dev1[] = {"",};
  unsigned int vip0_is_isp_used[2] = {1,1};
  unsigned int vip0_is_bayer_raw[2] = {1,0};
  unsigned int vip0_act_used[2] = {1,0};
  unsigned int vip0_act_addr[2] = {0x6c,0xff};
	unsigned char vip0_act_name_dev0[] = {"ov8825_act",};
  unsigned char vip0_act_name_dev1[] = {"",};

  unsigned int i2c_addr_vip1[2] = {0x78,0x42};
  unsigned char ccm_vip1_dev0[] = {"ov5650",};
  unsigned char ccm_vip1_dev1[] = {"gc0308",};
  unsigned int vip1_is_isp_used[2] = {1,1};
  unsigned int vip1_is_bayer_raw[2] = {1,0};
  unsigned int vip1_act_used[2] = {1,0};
  unsigned int vip1_act_addr[2] = {0x18,0xff};
	unsigned char vip1_act_name_dev0[] = {"ad5820",};
  unsigned char vip1_act_name_dev1[] = {"",};
#endif
  unsigned int i2c_addr[2], act_addr[2];
  unsigned int is_isp_used[2],is_bayer_raw[2],act_used[2];
  unsigned char *ccm_name[2],*act_name[2];
  unsigned int i;

  if(dev->id==0) {
    dev->dev_qty = 1;
    for(i = 0; i < 2; i++) {
	    is_isp_used[i] = vip0_is_isp_used[i];
	    is_bayer_raw[i] = vip0_is_bayer_raw[i];
	    i2c_addr[i] = i2c_addr_vip0[i];
	    act_used[i] = vip0_act_used[i];
	    act_addr[i] = vip0_act_addr[i];
  	}
    ccm_name[0] = ccm_vip0_dev0;
	  ccm_name[1] = ccm_vip0_dev1;
	  act_name[0] = vip0_act_name_dev0;
	  act_name[1] = vip0_act_name_dev1;
  } else if (dev->id == 1) {
    dev->dev_qty = 1;
    for(i = 0; i < 2; i++) {
	    is_isp_used[i] = vip1_is_isp_used[i];
	    is_bayer_raw[i] = vip1_is_bayer_raw[i];
	    i2c_addr[i] = i2c_addr_vip1[i];
	    act_used[i] = vip1_act_used[i];
	    act_addr[i] = vip1_act_addr[i];
  	}
    ccm_name[0] = ccm_vip1_dev0;
    ccm_name[1] = ccm_vip1_dev1;
    act_name[0] = vip1_act_name_dev0;
	  act_name[1] = vip1_act_name_dev1;
  }

  for(i=0; i<dev->dev_qty; i++)
  {
    dev->ccm_cfg[i]->twi_id = 1;
    type = strcmp(dev->ccm_cfg[i]->ccm,"");
    if((dev->ccm_cfg[i]->i2c_addr == 0xff)) //when insmod without parm
    {
		dev->ccm_cfg[i]->i2c_addr = i2c_addr[i];
		strcpy(dev->ccm_cfg[i]->ccm, ccm_name[i]);
		strcpy(dev->ccm_cfg[i]->isp_cfg_name, ccm_name[i]);

    }
    dev->ccm_cfg[i]->power.stby_mode = 0;
    dev->ccm_cfg[i]->vflip = 0;
    dev->ccm_cfg[i]->hflip = 0;
    dev->ccm_cfg[i]->is_isp_used = is_isp_used[i];
    dev->ccm_cfg[i]->is_bayer_raw = is_bayer_raw[i];
    dev->ccm_cfg[i]->act_used = act_used[i];
    strcpy(dev->ccm_cfg[i]->act_name , act_name[i]);
    dev->ccm_cfg[i]->act_slave = act_addr[i];
  }
#endif

  for(i=0; i<dev->dev_qty; i++)
  {
    vfe_dbg(0,"dev->ccm_cfg[%d]->ccm = %s\n",i,dev->ccm_cfg[i]->ccm);
    vfe_dbg(0,"dev->ccm_cfg[%d]->twi_id = %x\n",i,dev->ccm_cfg[i]->twi_id);
    vfe_dbg(0,"dev->ccm_cfg[%d]->i2c_addr = %x\n",i,dev->ccm_cfg[i]->i2c_addr);
		vfe_dbg(0,"dev->ccm_cfg[%d]->is_isp_used = %x\n",i,dev->ccm_cfg[i]->is_isp_used);
		vfe_dbg(0,"dev->ccm_cfg[%d]->is_bayer_raw = %x\n",i,dev->ccm_cfg[i]->is_bayer_raw);
    vfe_dbg(0,"dev->ccm_cfg[%d]->vflip = %x\n",i,dev->ccm_cfg[i]->vflip);
    vfe_dbg(0,"dev->ccm_cfg[%d]->hflip = %x\n",i,dev->ccm_cfg[i]->hflip);
    vfe_dbg(0,"dev->ccm_cfg[%d]->iovdd_str = %s\n",i,dev->ccm_cfg[i]->iovdd_str);
    vfe_dbg(0,"dev->ccm_cfg[%d]->avdd_str = %s\n",i,dev->ccm_cfg[i]->avdd_str);
    vfe_dbg(0,"dev->ccm_cfg[%d]->dvdd_str = %s\n",i,dev->ccm_cfg[i]->dvdd_str);
    vfe_dbg(0,"dev->ccm_cfg[%d]->afvdd_str = %s\n",i,dev->ccm_cfg[i]->afvdd_str);
    vfe_dbg(0,"dev->ccm_cfg[%d]->act_used = %d\n",i,dev->ccm_cfg[i]->act_used);
    vfe_dbg(0,"dev->ccm_cfg[%d]->act_name = %s\n",i,dev->ccm_cfg[i]->act_name);
    vfe_dbg(0,"dev->ccm_cfg[%d]->act_slave = 0x%x\n",i,dev->ccm_cfg[i]->act_slave);
  }

  return 0;
}

struct isp_init_config isp_init_def_cfg = {
    .isp_test_settings =
    {
      /*isp test param */
      .isp_test_mode       = 1,
      .isp_test_exptime    = 0,
      .exp_line_start        = 1000  ,
      .exp_line_step         = 16    ,
      .exp_line_end          = 10000 ,
      .exp_change_interval   = 5     ,

      .isp_test_gain         = 0     ,
      .gain_start            = 16    ,
      .gain_step             = 1     ,
      .gain_end              = 256   ,
      .gain_change_interval  = 3     ,

      .isp_test_focus        = 0     ,
      .focus_start           = 0     ,
      .focus_step            = 10    ,
      .focus_end             = 800   ,
      .focus_change_interval = 2     ,

      .isp_dbg_level       = 0,
      .isp_focus_len       = 0,
      .isp_gain               = 64,
      .isp_exp_line        = 7680,

      .isp_color_temp        = 6500,
      .ae_forced = 0,
      .lum_forced = 40,
      /*isp enable param */
      .sprite_en            = 0,
      .lsc_en               = 0,
      .ae_en                = 0,
      .af_en                = 0,
      .awb_en               = 0,
      .drc_en               = 0,
      .defog_en             = 0,
      .satur_en             = 0,
      .tdf_en             = 0,
      .pri_contrast_en      = 0,
      .hdr_gamma_en         = 0,
      .disc_en      = 0,
      .linear_en         = 0,
    },

    .isp_3a_settings =
    {
      /*isp ae param */
      .define_ae_table           = 0,
      .ae_max_lv                  = 1800,
      .fno                             = 280,
      .ae_hist_mod_en            = 0,
      .ae_window_overexp_weigth = 16,
      .ae_hist_overexp_weight   = 32,
      .ae_video_speed           = 4,
      .ae_capture_speed         = 8,
      .ae_tolerance             = 6,
      .ae_min_frame_rate        = 8,
      .exp_delay_frame          = 2,
      .gain_delay_frame         = 2,
      .exp_comp_step 		= 8,
      .adaptive_frame_rate      = 1,
      .high_quality_mode_en     = 0,
      .force_frame_rate         = 0,

      /*isp awb param */
      .awb_interval                 = 4,
      .awb_speed = 8,
      //.awb_mode_select          = 1,
      .awb_color_temper_low     = 2500,
      .awb_color_temper_high    = 7500,
      .awb_skin_color_num = 0,
      .vcm_min_code             = 0,
      .vcm_max_code             = 650,
      .af_interval_time = 100,
	.af_speed_ind = 4, //0~5
	.af_auto_fine_en = 1,
	.af_single_fine_en = 0,
	.af_fine_step = 24,
	.af_move_cnt = 4,
	.af_still_cnt = 1,
    },
    .isp_tunning_settings =
    {
      .flash_gain = 80,
      .flash_delay_frame = 8,
      .flicker_type = 1,
      .flicker_ratio = 20,
      /*isp_dpc_otf_param*/
      .dpc_th_slop               = 4,
      .dpc_otf_min_th            = 16,
      .dpc_otf_max_th            = 1024,

      .front_camera = 0,
      .defog_value   =200,
      .hor_visual_angle = 60,
      .ver_visual_angle  = 60,
      .focus_length   = 425,
      .use_bright_contrast = 0,
      .low_bright_supp      = 324,
      .low_bright_drc       = 24,
      .color_denoise_level  = 0,

      /*isp tune param */
      .bayer_gain_offset = {256,256,256,256,0,0,0,0},
      .csc_coeff = {1024,1024,1024,1024,1024,1024},
      .lsc_mod = 0,
      .gamma_num = 1,
      .lsc_center =  {2048,2048},
      .lsc_tbl = {{0},{0},{0},{0},{0},{0},{0},{0}},
      .hdr_tbl = {{0},{0},{0},{0}},
      .gamma_tbl = {10,20,30,40,50,60,70,80},
      .color_matrix_ini[0] =
      {
        .matrix = {{256,0,0},{0,256,0},{0,0,256}},
        .offset = {0, 0, 0},
      },
	.color_matrix_ini[1] =
      {
        .matrix = {{256,0,0},{0,256,0},{0,0,256}},
        .offset = {0, 0, 0},
      },
	.color_matrix_ini[2] =
      {
        .matrix = {{256,0,0},{0,256,0},{0,0,256}},
        .offset = {0, 0, 0},
      },
    },
};



void set_isp_test_mode(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.isp_test_mode = *(int *)value; }
void set_isp_test_exptime(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.isp_test_exptime = *(int *)value;}
void set_exp_line_start(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.exp_line_start      = *(int *)value; }
void set_exp_line_step(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.exp_line_step       = *(int *)value; }
void set_exp_line_end(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.exp_line_end        = *(int *)value; }
void set_exp_change_interval(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.exp_change_interval = *(int *)value; }

void set_isp_test_gain(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.isp_test_gain = *(int *)value; }
void set_gain_start(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.gain_start           = *(int *)value; }
void set_gain_step(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.gain_step            = *(int *)value; }
void set_gain_end (struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.gain_end             = *(int *)value; }
void set_gain_change_interval(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.gain_change_interval = *(int *)value; }

void set_isp_test_focus(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.isp_test_focus        = *(int *)value; }
void set_focus_start(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.focus_start           = *(int *)value; }
void set_focus_step(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.focus_step            = *(int *)value; }
void set_focus_end(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.focus_end             = *(int *)value; }
void set_focus_change_interval(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.focus_change_interval = *(int *)value; }

void set_isp_dbg_level(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.isp_dbg_level = *(int *)value; }
void set_isp_focus_len(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.isp_focus_len = *(int *)value; }
void set_isp_gain(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.isp_gain = *(int *)value; }
void set_isp_exp_line(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.isp_exp_line = *(int *)value; }
void set_isp_color_temp(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.isp_color_temp = *(int *)value; }

void set_ae_forced(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.ae_forced = *(int *)value; }
void set_lum_forced(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.lum_forced  = *(int *)value; }
void set_sprite_en(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.sprite_en = *(int *)value; }
void set_lsc_en(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.lsc_en  = *(int *)value; }
void set_ae_en (struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.ae_en  = *(int *)value; }
void set_af_en (struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.af_en  = *(int *)value; }
void set_awb_en(struct isp_init_config *isp_ini_cfg, void *value, int len){ isp_ini_cfg->isp_test_settings.awb_en  = *(int *)value; }
void set_drc_en(struct isp_init_config *isp_ini_cfg, void *value, int len)  { isp_ini_cfg->isp_test_settings.drc_en  = *(int *)value; }
void set_defog_en(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.defog_en = *(int *)value; }
void set_satur_en(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.satur_en = *(int *)value; }
void set_tdf_en(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.tdf_en = *(int *)value; }
void set_pri_contrast_en(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.pri_contrast_en = *(int *)value; }
void set_hdr_gamma_en(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.hdr_gamma_en = *(int *)value; }
void set_disc_en(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.disc_en = *(int *)value; }
void set_linear_en(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_test_settings.linear_en = *(int *)value; }
void set_define_ae_table(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.define_ae_table = *(int *)value; }

void set_ae_table_preview_length(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.ae_table_preview_length = *(int *)value; }
void set_ae_table_capture_length(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.ae_table_capture_length = *(int *)value; }
void set_ae_table_video_length(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.ae_table_video_length = *(int *)value; }

void set_ae_max_lv(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.ae_max_lv = *(int *)value; }
void set_fno(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.fno = *(int *)value; }
void set_ae_hist_mod_en(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.ae_hist_mod_en = *(int *)value; }
void set_ae_window_overexp_weigth(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.ae_window_overexp_weigth = *(int *)value; }
void set_ae_hist_overexp_weight(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.ae_hist_overexp_weight = *(int *)value; }
void set_ae_video_speed(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.ae_video_speed = *(int *)value; }
void set_ae_capture_speed(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.ae_capture_speed = *(int *)value; }
void set_ae_tolerance(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.ae_tolerance = *(int *)value; }
void set_ae_min_frame_rate(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.ae_min_frame_rate = *(int *)value; }
void set_exp_delay_frame(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.exp_delay_frame = *(int *)value; }
void set_gain_delay_frame(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.gain_delay_frame = *(int *)value; }
void set_exp_comp_step(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.exp_comp_step = *(int *)value; }

void set_high_quality_mode_en(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.high_quality_mode_en = *(int *)value; }
void set_adaptive_frame_rate(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.adaptive_frame_rate = *(int *)value; }
void set_force_frame_rate(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.force_frame_rate = *(int *)value; }

void set_awb_interval(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.awb_interval = *(int *)value; }
void set_awb_speed(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.awb_speed = *(int *)value; }

//void set_awb_mode_select(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.awb_mode_select = *(int *)value; }
//void set_awb_tolerance(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.awb_tolerance = *(int *)value; }

void set_awb_color_temper_low(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.awb_color_temper_low = *(int *)value; }
void set_awb_color_temper_high(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.awb_color_temper_high = *(int *)value; }
//void set_r_gain_2900k(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.r_gain_2900k = *(int *)value; }
//void set_b_gain_2900k(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.b_gain_2900k = *(int *)value; }
void set_vcm_min_code(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.vcm_min_code = *(int *)value; }
void set_vcm_max_code(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.vcm_max_code = *(int *)value; }
void set_af_interval_time(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.af_interval_time = *(int *)value; }

void set_af_speed_ind(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.af_speed_ind = *(int *)value; }
void set_af_auto_fine_en(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.af_auto_fine_en = *(int *)value; }
void set_af_single_fine_en(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.af_single_fine_en = *(int *)value; }
void set_af_fine_step(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.af_fine_step = *(int *)value; }


void set_af_move_cnt(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.af_move_cnt = *(int *)value; }
void set_af_still_cnt(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.af_still_cnt = *(int *)value; }


void set_flash_gain(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_tunning_settings.flash_gain = *(int *)value; }
void set_flash_delay_frame(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_tunning_settings.flash_delay_frame = *(int *)value; }

void set_flicker_type(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_tunning_settings.flicker_type = *(int *)value; }
void set_flicker_ratio(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_tunning_settings.flicker_ratio = *(int *)value; }

void set_dpc_th_slop(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_tunning_settings.dpc_th_slop = *(int *)value; }
void set_dpc_otf_min_th(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_tunning_settings.dpc_otf_min_th = *(int *)value; }
void set_dpc_otf_max_th(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_tunning_settings.dpc_otf_max_th = *(int *)value; }
void set_front_camera(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_tunning_settings.front_camera = *(int *)value; }
void set_defog_value(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_tunning_settings.defog_value = *(int *)value; }

void set_hor_visual_angle(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_tunning_settings.hor_visual_angle = *(int *)value; }
void set_ver_visual_angle(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_tunning_settings.ver_visual_angle = *(int *)value; }
void set_focus_length(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_tunning_settings.focus_length = *(int *)value; }

void set_use_bright_contrast(struct isp_init_config *isp_ini_cfg, void *value, int len)  { isp_ini_cfg->isp_tunning_settings.use_bright_contrast= *(int *)value; }
void set_low_bright_supp(struct isp_init_config *isp_ini_cfg, void *value, int len)  { isp_ini_cfg->isp_tunning_settings.low_bright_supp = *(int *)value; }
void set_low_bright_drc(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_tunning_settings.low_bright_drc = *(int *)value; }
void set_color_denoise_level(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_tunning_settings.color_denoise_level = *(int *)value; }
void set_lsc_mod(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_tunning_settings.lsc_mod = *(int *)value; }
void set_lsc_center_x(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_tunning_settings.lsc_center[0] = *(int *)value; }
void set_lsc_center_y(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_tunning_settings.lsc_center[1] = *(int *)value; }
void set_gamma_num(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_tunning_settings.gamma_num = *(int *)value; }

//for new awb
void set_awb_light_num(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.awb_light_num = *(int *)value; }
void set_awb_ext_light_num(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.awb_ext_light_num = *(int *)value; }
void set_awb_skin_color_num(struct isp_init_config *isp_ini_cfg, void *value, int len) { isp_ini_cfg->isp_3a_settings.awb_skin_color_num = *(int *)value; }

void set_awb_light_info(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int i,*tmp;
	tmp = (int *)value;
	for(i = 0; i < len; i++)
	{
		isp_ini_cfg->isp_3a_settings.awb_light_info[i] = tmp[i];
	}
}
void set_awb_ext_light_info(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int i,*tmp;
	tmp = (int *)value;
	for(i = 0; i < len; i++)
	{
		isp_ini_cfg->isp_3a_settings.awb_ext_light_info[i] = tmp[i];
	}
}
void set_awb_skin_color_info(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int i,*tmp;
	tmp = (int *)value;
	for(i = 0; i < len; i++)
	{
		isp_ini_cfg->isp_3a_settings.awb_skin_color_info[i] = tmp[i];
	}
}

void set_awb_preset_gain(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int i,*tmp;
	tmp = (int *)value;
	for(i = 0; i < len; i++)
	{
		isp_ini_cfg->isp_3a_settings.awb_preset_gain[i] = tmp[i];
	}
}


void set_ae_table_preview(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int i,*tmp;
	tmp = (int *)value;
	for(i = 0; i < len; i++)
	{
		isp_ini_cfg->isp_3a_settings.ae_table_preview[i] = tmp[i];
	}
}
void set_ae_table_capture(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int i,*tmp;
	tmp = (int *)value;
	for(i = 0; i < len; i++)
	{
		isp_ini_cfg->isp_3a_settings.ae_table_capture[i] = tmp[i];
	}
}
void set_ae_table_video(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int i,*tmp;
	tmp = (int *)value;
	for(i = 0; i < len; i++)
	{
		isp_ini_cfg->isp_3a_settings.ae_table_video[i] = tmp[i];
	}
}
void set_ae_win_weight(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int i,*tmp;
	tmp = (int *)value;
	for(i = 0; i < len; i++)
	{
		isp_ini_cfg->isp_3a_settings.ae_win_weight[i] = tmp[i];
	}
}

/*
void set_awb_light_param(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int i,*tmp;
	tmp = (int *)value;
	for(i = 0; i < len; i++)
	{
		isp_ini_cfg->isp_3a_settings.awb_light_param[i] = tmp[i];
	}
}

void set_awb_coeff(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int i,*tmp;
	tmp = (int *)value;
	for(i = 0; i < len; i++)
	{
		isp_ini_cfg->isp_3a_settings.awb_coeff[i] = tmp[i];
	}
}
*/
void set_isp_iso_100_cfg(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int i,*tmp, *cfg_pt;
	tmp = (int *)value;
	cfg_pt = &isp_ini_cfg->isp_iso_settings.isp_iso_100_cfg.sharp_coeff[0];
	for(i = 0; i < len; i++)
	{
		cfg_pt[i] = tmp[i];
	}
}
void set_isp_iso_200_cfg(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int i,*tmp, *cfg_pt;
	tmp = (int *)value;
	cfg_pt = &isp_ini_cfg->isp_iso_settings.isp_iso_200_cfg.sharp_coeff[0];
	for(i = 0; i < len; i++)
	{
		cfg_pt[i] = tmp[i];
	}
}
void set_isp_iso_400_cfg(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int i,*tmp, *cfg_pt;
	tmp = (int *)value;
	cfg_pt = &isp_ini_cfg->isp_iso_settings.isp_iso_400_cfg.sharp_coeff[0];
	for(i = 0; i < len; i++)
	{
		cfg_pt[i] = tmp[i];
	}
}
void set_isp_iso_800_cfg(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int i,*tmp, *cfg_pt;
	tmp = (int *)value;
	cfg_pt = &isp_ini_cfg->isp_iso_settings.isp_iso_800_cfg.sharp_coeff[0];
	for(i = 0; i < len; i++)
	{
		cfg_pt[i] = tmp[i];
	}
}
void set_isp_iso_1600_cfg(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int i,*tmp, *cfg_pt;
	tmp = (int *)value;
	cfg_pt = &isp_ini_cfg->isp_iso_settings.isp_iso_1600_cfg.sharp_coeff[0];
	for(i = 0; i < len; i++)
	{
		cfg_pt[i] = tmp[i];
	}
}
void set_isp_iso_3200_cfg(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int i,*tmp, *cfg_pt;
	tmp = (int *)value;
	cfg_pt = &isp_ini_cfg->isp_iso_settings.isp_iso_3200_cfg.sharp_coeff[0];
	for(i = 0; i < len; i++)
	{
		cfg_pt[i] = tmp[i];
	}
}

void set_color_matrix0(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int *tmp = (int *)value;
	struct isp_rgb2rgb_gain_offset *color_matrix = &isp_ini_cfg->isp_tunning_settings.color_matrix_ini[0];
	color_matrix->matrix[0][0] = tmp[0];
	color_matrix->matrix[0][1] = tmp[1];
	color_matrix->matrix[0][2] = tmp[2];
	color_matrix->matrix[1][0] = tmp[3];
	color_matrix->matrix[1][1] = tmp[4];
	color_matrix->matrix[1][2] = tmp[5];
	color_matrix->matrix[2][0] = tmp[6];
	color_matrix->matrix[2][1] = tmp[7];
	color_matrix->matrix[2][2] = tmp[8];
	color_matrix->offset[0] = tmp[9];
	color_matrix->offset[1] = tmp[10];
	color_matrix->offset[2] = tmp[11];
}
void set_color_matrix1(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int *tmp = (int *)value;
	struct isp_rgb2rgb_gain_offset *color_matrix = &isp_ini_cfg->isp_tunning_settings.color_matrix_ini[1];
	color_matrix->matrix[0][0] = tmp[0];
	color_matrix->matrix[0][1] = tmp[1];
	color_matrix->matrix[0][2] = tmp[2];
	color_matrix->matrix[1][0] = tmp[3];
	color_matrix->matrix[1][1] = tmp[4];
	color_matrix->matrix[1][2] = tmp[5];
	color_matrix->matrix[2][0] = tmp[6];
	color_matrix->matrix[2][1] = tmp[7];
	color_matrix->matrix[2][2] = tmp[8];
	color_matrix->offset[0] = tmp[9];
	color_matrix->offset[1] = tmp[10];
	color_matrix->offset[2] = tmp[11];
}
void set_color_matrix2(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int *tmp = (int *)value;
	struct isp_rgb2rgb_gain_offset *color_matrix = &isp_ini_cfg->isp_tunning_settings.color_matrix_ini[2];
	color_matrix->matrix[0][0] = tmp[0];
	color_matrix->matrix[0][1] = tmp[1];
	color_matrix->matrix[0][2] = tmp[2];
	color_matrix->matrix[1][0] = tmp[3];
	color_matrix->matrix[1][1] = tmp[4];
	color_matrix->matrix[1][2] = tmp[5];
	color_matrix->matrix[2][0] = tmp[6];
	color_matrix->matrix[2][1] = tmp[7];
	color_matrix->matrix[2][2] = tmp[8];
	color_matrix->offset[0] = tmp[9];
	color_matrix->offset[1] = tmp[10];
	color_matrix->offset[2] = tmp[11];
}
/*
void set_color_matrix_inv(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int *tmp = (int *)value;
	struct isp_rgb2rgb_gain_offset *color_matrix = &isp_ini_cfg->isp_3a_settings.color_matrix_inv;
	color_matrix->matrix[0][0] = tmp[0];
	color_matrix->matrix[0][1] = tmp[1];
	color_matrix->matrix[0][2] = tmp[2];
	color_matrix->matrix[1][0] = tmp[3];
	color_matrix->matrix[1][1] = tmp[4];
	color_matrix->matrix[1][2] = tmp[5];
	color_matrix->matrix[2][0] = tmp[6];
	color_matrix->matrix[2][1] = tmp[7];
	color_matrix->matrix[2][2] = tmp[8];
	color_matrix->offset[0] = tmp[9];
	color_matrix->offset[1] = tmp[10];
	color_matrix->offset[2] = tmp[11];
}*/

void set_isp_gain_offset(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int i,*tmp;
	tmp = (int *)value;
	for(i = 0; i < len; i++)
	{
		isp_ini_cfg->isp_tunning_settings.bayer_gain_offset[i] = tmp[i];
	}
}
void set_isp_csc(struct isp_init_config *isp_ini_cfg, void *value, int len)
{
	int i,*tmp;
	tmp = (int *)value;
	for(i = 0; i < len; i++)
	{
		isp_ini_cfg->isp_tunning_settings.csc_coeff[i] = tmp[i];
	}
}

struct IspParamAttribute
{
	char *main;
	char *sub;
	int len;
	void (*set_param)(struct isp_init_config *, void *, int len);
};

struct FileAttribute
{
	char *file_name;
	int param_len;
	struct IspParamAttribute *pIspParam;
};

static struct IspParamAttribute IspTestParam[] =
{
	{ "isp_test_cfg", "isp_test_mode"        , 1 ,  set_isp_test_mode         ,},
	{ "isp_test_cfg", "isp_test_exptime"     , 1 ,  set_isp_test_exptime      ,},
	{ "isp_test_cfg", "exp_line_start"       , 1 ,  set_exp_line_start        ,},
	{ "isp_test_cfg", "exp_line_step"        , 1 ,  set_exp_line_step         ,},
	{ "isp_test_cfg", "exp_line_end"         , 1 ,  set_exp_line_end          ,},
	{ "isp_test_cfg", "exp_change_interval"  , 1 ,  set_exp_change_interval   ,},
	{ "isp_test_cfg", "isp_test_gain"        , 1 ,  set_isp_test_gain         ,},
	{ "isp_test_cfg", "gain_start"           , 1 ,  set_gain_start            ,},
	{ "isp_test_cfg", "gain_step"            , 1 ,  set_gain_step             ,},
	{ "isp_test_cfg", "gain_end"             , 1 ,  set_gain_end              ,},
	{ "isp_test_cfg", "gain_change_interval" , 1 ,  set_gain_change_interval  ,},
	{ "isp_test_cfg", "isp_test_focus"       , 1 ,  set_isp_test_focus        ,},
	{ "isp_test_cfg", "focus_start"          , 1 ,  set_focus_start           ,},
	{ "isp_test_cfg", "focus_step"           , 1 ,  set_focus_step            ,},
	{ "isp_test_cfg", "focus_end"            , 1 ,  set_focus_end             ,},
	{ "isp_test_cfg", "focus_change_interval", 1 ,  set_focus_change_interval ,},

	{ "isp_test_cfg", "isp_dbg_level",    1 ,  set_isp_dbg_level    ,},
	{ "isp_test_cfg", "isp_focus_len",    1 ,  set_isp_focus_len    ,},
	{ "isp_test_cfg", "isp_gain",         1 ,  set_isp_gain         ,},
	
	{ "isp_test_cfg", "isp_exp_line",     1 ,  set_isp_exp_line     ,},
	{ "isp_test_cfg", "isp_color_temp",         1 ,  set_isp_color_temp         ,},
	{ "isp_test_cfg", "ae_forced",     1 ,  set_ae_forced     ,},
	{ "isp_test_cfg", "lum_forced",         1 ,  set_lum_forced         ,},

	{ "isp_en_cfg",   "sprite_en",        1 ,  set_sprite_en        ,},
	{ "isp_en_cfg",   "lsc_en",           1 ,  set_lsc_en           ,},
	{ "isp_en_cfg",   "ae_en",            1 ,  set_ae_en            ,},
	{ "isp_en_cfg",   "af_en",            1 ,  set_af_en            ,},
	{ "isp_en_cfg",   "awb_en",           1 ,  set_awb_en           ,},
	{ "isp_en_cfg",   "drc_en",           1 ,  set_drc_en           ,},
	{ "isp_en_cfg",   "defog_en",         1 ,  set_defog_en         ,},
	{ "isp_en_cfg",   "satur_en",         1 ,  set_satur_en         ,},
	{ "isp_en_cfg",   "tdf_en",		  1 ,  set_tdf_en 		,},

	{ "isp_en_cfg",   "pri_contrast_en",  1 ,  set_pri_contrast_en  ,},
	{ "isp_en_cfg",   "hdr_gamma_en",     1 ,  set_hdr_gamma_en     ,},
	{ "isp_en_cfg",   "disc_en",         1 ,  set_disc_en         ,},
	{ "isp_en_cfg",   "linear_en",		  1 ,  set_linear_en 		,},
};

static struct IspParamAttribute Isp3aParam[] =
{
	{ "isp_ae_cfg",   "define_ae_table",          1 ,  set_define_ae_table          ,},

	{ "isp_ae_cfg",   "ae_max_lv",          1 ,  set_ae_max_lv,},
	{ "isp_ae_cfg",   "fno"            ,          1 ,  set_fno                      ,},

	{ "isp_ae_cfg",   "ae_table_preview_length",          1 ,  set_ae_table_preview_length          ,},
	{ "isp_ae_cfg",   "ae_table_capture_length",          1 ,  set_ae_table_capture_length          ,},
	{ "isp_ae_cfg",   "ae_table_video_length",          1 ,  set_ae_table_video_length          ,},

	{ "isp_ae_cfg",   "ae_table_preview_",          28 ,  set_ae_table_preview           ,},
	{ "isp_ae_cfg",   "ae_table_capture_",          28 ,  set_ae_table_capture           ,},
	{ "isp_ae_cfg",   "ae_table_video_",          28 ,  set_ae_table_video           ,},
	{ "isp_ae_cfg",   "ae_win_weight_",          64 ,  set_ae_win_weight           ,},

	{ "isp_ae_cfg",   "ae_hist_mod_en",            1 ,  set_ae_hist_mod_en            ,},
	{ "isp_ae_cfg",   "ae_window_overexp_weigth", 1 ,  set_ae_window_overexp_weigth ,},
	{ "isp_ae_cfg",   "ae_hist_overexp_weight",   1 ,  set_ae_hist_overexp_weight   ,},
	{ "isp_ae_cfg",   "ae_video_speed",           1 ,  set_ae_video_speed           ,},
	{ "isp_ae_cfg",   "ae_capture_speed",         1 ,  set_ae_capture_speed         ,},
	{ "isp_ae_cfg",   "ae_tolerance",             1 ,  set_ae_tolerance             ,},
	{ "isp_ae_cfg",   "ae_min_frame_rate",        1 ,  set_ae_min_frame_rate        ,},
	{ "isp_ae_cfg",   "exp_delay_frame",          1 ,  set_exp_delay_frame          ,},
	{ "isp_ae_cfg",   "gain_delay_frame",         1 ,  set_gain_delay_frame         ,},
	{ "isp_ae_cfg",   "exp_comp_step",         1 ,  set_exp_comp_step         ,},


	{ "isp_ae_cfg",   "high_quality_mode_en",     1 , set_high_quality_mode_en      ,},
	{ "isp_ae_cfg",   "adaptive_frame_rate",      1 , set_adaptive_frame_rate       ,},
	{ "isp_ae_cfg",   "force_frame_rate",         1 , set_force_frame_rate          ,},

	{ "isp_awb_cfg",  "awb_interval",          1 ,  set_awb_interval          ,},
	{ "isp_awb_cfg",  "awb_speed",          1 ,  set_awb_speed	,},


	{ "isp_awb_cfg",  "awb_color_temper_low",     1 ,  set_awb_color_temper_low     ,},
	{ "isp_awb_cfg",  "awb_color_temper_high",    1 ,  set_awb_color_temper_high    ,},

	{ "isp_awb_cfg",  "awb_light_num",    1 ,  set_awb_light_num    ,},
	{ "isp_awb_cfg",  "awb_ext_light_num",             1 ,  set_awb_ext_light_num             ,},
	{ "isp_awb_cfg",  "awb_skin_color_num",             1 ,  set_awb_skin_color_num             ,},
	{ "isp_awb_cfg",  "awb_light_info_",             100 ,  set_awb_light_info           ,},
	{ "isp_awb_cfg",  "awb_ext_light_info_",             60 ,  set_awb_ext_light_info             ,},
	{ "isp_awb_cfg",  "awb_skin_color_info_",             40 ,  set_awb_skin_color_info             ,},
	{ "isp_awb_cfg",  "awb_perset_gain_",             22 ,  set_awb_preset_gain             ,},

	{ "isp_af_cfg",   "vcm_min_code",             1 ,  set_vcm_min_code             ,},
	{ "isp_af_cfg",   "vcm_max_code",             1 ,  set_vcm_max_code             ,},
	{ "isp_af_cfg",   "af_interval_time",             1 ,  set_af_interval_time             ,},

	{ "isp_af_cfg",   "af_speed_ind",             1 ,  set_af_speed_ind             ,},
	{ "isp_af_cfg",   "af_auto_fine_en",             1 ,  set_af_auto_fine_en             ,},
	{ "isp_af_cfg",   "af_single_fine_en",             1 ,  set_af_single_fine_en             ,},
	{ "isp_af_cfg",   "af_fine_step",             1 ,  set_af_fine_step             ,},

	
	{ "isp_af_cfg",   "af_move_cnt",             1 ,  set_af_move_cnt             ,},
	{ "isp_af_cfg",   "af_still_cnt",             1 ,  set_af_still_cnt             ,},
};
static struct IspParamAttribute IspIsoParam[] =
{
	{ "isp_iso_100_cfg" ,      "iso_param_",      48,  set_isp_iso_100_cfg ,},
	{ "isp_iso_200_cfg" ,      "iso_param_",      48,  set_isp_iso_200_cfg ,},
	{ "isp_iso_400_cfg" ,      "iso_param_",      48,  set_isp_iso_400_cfg ,},
	{ "isp_iso_800_cfg" ,      "iso_param_",      48,  set_isp_iso_800_cfg ,},
	{ "isp_iso_1600_cfg" ,     "iso_param_",     48,  set_isp_iso_1600_cfg,},
	{ "isp_iso_3200_cfg" ,     "iso_param_",     48,  set_isp_iso_3200_cfg,},
};
static struct IspParamAttribute IspTuningParam[] =
{
	{ "isp_drc_cfg" ,           "use_bright_contrast",    1 ,  set_use_bright_contrast    ,},
	{ "isp_drc_cfg" ,           "low_bright_supp",    1 ,  set_low_bright_supp    ,},
	{ "isp_drc_cfg" ,           "low_bright_drc",     1 ,  set_low_bright_drc     ,},
	{ "isp_tuning_cfg" ,        "color_denoise_level",1 ,  set_color_denoise_level,},
	{ "isp_tuning_cfg" ,        "flash_gain",         1 ,  set_flash_gain         ,},
	{ "isp_tuning_cfg" ,        "flash_delay_frame",         1 ,  set_flash_delay_frame         ,},
	
	{ "isp_dpc_cfg" ,        "dpc_th_slop",1 ,  set_dpc_th_slop,},
	{ "isp_dpc_cfg" ,        "dpc_otf_min_th",         1 ,  set_dpc_otf_min_th         ,},
	{ "isp_dpc_cfg" ,        "dpc_otf_max_th",         1 ,  set_dpc_otf_max_th         ,},

	{ "isp_tuning_cfg" ,        "flicker_type",       1 ,  set_flicker_type       ,},
	{ "isp_tuning_cfg" ,        "flicker_ratio",       1 ,  set_flicker_ratio       ,},
	{ "isp_tuning_cfg" ,        "front_camera",       1 ,  set_front_camera       ,},
	{ "isp_tuning_cfg" ,        "defog_value",        1 ,  set_defog_value        ,},
	{ "isp_tuning_cfg" ,        "hor_visual_angle",       1 ,  set_hor_visual_angle       ,},
	{ "isp_tuning_cfg" ,        "ver_visual_angle",       1 ,  set_ver_visual_angle       ,},
	{ "isp_tuning_cfg" ,        "focus_length",        1 ,  set_focus_length        ,},
	{ "isp_tuning_cfg" ,        "gamma_num",        1 ,  set_gamma_num        ,},

	{ "isp_lsc" ,               "lsc_mod",       1 ,  set_lsc_mod       ,},
	{ "isp_lsc" ,               "lsc_center_x",       1 ,  set_lsc_center_x       ,},
	{ "isp_lsc" ,               "lsc_center_y",       1 ,  set_lsc_center_y       ,},
	{ "isp_gain_offset" ,       "gain_offset_",       8 ,  set_isp_gain_offset    ,},
	{ "isp_csc" ,               "csc_coeff_",         6 ,  set_isp_csc            ,},
	{ "isp_color_matrix0" ,      "matrix_",            12 ,  set_color_matrix0      ,},
	{ "isp_color_matrix1" ,      "matrix_",            12 ,  set_color_matrix1      ,},
	{ "isp_color_matrix2" ,      "matrix_",            12 ,  set_color_matrix2      ,},

};

static struct FileAttribute FileAttr [] =
{
	 { "isp_test_param.ini",    ARRAY_SIZE(IspTestParam)  , &IspTestParam[0],  },
	 { "isp_3a_param.ini",      ARRAY_SIZE(Isp3aParam)    , &Isp3aParam[0],    },
	 { "isp_iso_param.ini",     ARRAY_SIZE(IspIsoParam)   , &IspIsoParam[0],   },
	 { "isp_tuning_param.ini",  ARRAY_SIZE(IspTuningParam), &IspTuningParam[0],},
};

int fetch_isp_cfg(struct isp_init_config *isp_ini_cfg, struct cfg_section *cfg_section, struct FileAttribute *file_attr)
{
	int i, j, *array_value;
	struct cfg_subkey subkey;
	struct IspParamAttribute *param;
	char sub_name[128] = {0};
	/* fetch ISP isp_test_mode! */
	for (i = 0; i < file_attr->param_len;  i++)
	{
		param = file_attr->pIspParam + i;
		if(param->main == NULL || param->sub == NULL)
		{
			vfe_warn("param->main or param->sub is NULL!\n");
			continue;
		}
		if(param->len == 1)
		{
			if (CFG_ITEM_VALUE_TYPE_INT != cfg_get_one_subkey(cfg_section,param->main, param->sub, &subkey))
			{
				vfe_dbg(0,"Warning: %s->%s,apply default value!\n",param->main, param->sub);
			}
			else
			{
				if(param->set_param)
				{
					param->set_param(isp_ini_cfg, (void *)&subkey.value.val, param->len);
					vfe_dbg(0,"fetch_isp_cfg_single: %s->%s  = %d\n",param->main, param->sub,subkey.value.val);
				}
			}
		}
		else if(param->len > 1)
		{
			if(!strcmp(param->sub, "awb_light_info_"))
			{
				param->len = 10 * isp_ini_cfg->isp_3a_settings.awb_light_num;
			}
			if(!strcmp(param->sub, "awb_ext_light_info_"))
			{
				param->len = 10 * isp_ini_cfg->isp_3a_settings.awb_ext_light_num;
			}
			if(!strcmp(param->sub, "awb_skin_color_info_"))
			{
				param->len = 10 * isp_ini_cfg->isp_3a_settings.awb_skin_color_num;
			}

			array_value = (int*)vmalloc(param->len*sizeof(int));
			memset(array_value, 0, param->len*sizeof(int));
			for(j = 0;j<param->len;j++)
			{
				sprintf(sub_name, "%s%d",param->sub, j);
				if (CFG_ITEM_VALUE_TYPE_INT != cfg_get_one_subkey(cfg_section,param->main,sub_name,&subkey))
				{
					if(strcmp(param->sub, "iso_param_"))
						vfe_warn("fetch %s from %s failed, set %s = 0!\n",sub_name,param->main, sub_name);
					array_value[j] = 0;
				}
				else
				{
					array_value[j] = subkey.value.val;
					vfe_dbg(0,"fetch_isp_cfg_array: %s->%s  = %d\n",param->main, sub_name, subkey.value.val);
				}
			}
			if(param->set_param)
			{
				param->set_param(isp_ini_cfg, (void *)array_value, param->len);
			}

			if(array_value)
				vfree(array_value);
		}
	}
	 vfe_dbg(0,"fetch isp_cfg done!\n");
	return 0;
}
int fetch_isp_tbl(struct isp_init_config *isp_ini_cfg, char* tbl_patch)
{
	int len, ret = 0;
	char isp_gamma_tbl_path[128] = "\0",isp_hdr_tbl_path[128] = "\0",isp_lsc_tbl_path[128] = "\0";
	char *buf;
	strcpy(isp_gamma_tbl_path, tbl_patch);
	strcpy(isp_hdr_tbl_path, tbl_patch);
	strcpy(isp_lsc_tbl_path, tbl_patch);

	strcat(isp_gamma_tbl_path, "gamma_tbl.bin");
	strcat(isp_hdr_tbl_path, "hdr_tbl.bin");
	strcat(isp_lsc_tbl_path, "lsc_tbl.bin");

	vfe_print("Fetch table form \"%s\", gamma num = %d\n",isp_gamma_tbl_path, isp_ini_cfg->isp_tunning_settings.gamma_num);
	buf = (char*)kzalloc(SIZE_OF_LSC_TBL_MOD1,GFP_KERNEL);

	/* fetch gamma_tbl table! */
	if(isp_ini_cfg->isp_tunning_settings.gamma_num > 1 && isp_ini_cfg->isp_tunning_settings.gamma_num <= 5 )
		len = cfg_read_file(isp_gamma_tbl_path,buf, ISP_GAMMA_MEM_SIZE * isp_ini_cfg->isp_tunning_settings.gamma_num);
	else
		len = cfg_read_file(isp_gamma_tbl_path,buf, ISP_GAMMA_MEM_SIZE);
	if(len < 0)
	{
		vfe_warn("read gamma_tbl from gamma_tbl.bin failed!\n");
		ret =  -1;
	}
	else
	{
		if(isp_ini_cfg->isp_tunning_settings.gamma_num > 1){
		
			memcpy(isp_ini_cfg->isp_tunning_settings.gamma_tbl_ini, buf, len);
			memcpy(isp_ini_cfg->isp_tunning_settings.gamma_tbl, buf, ISP_GAMMA_MEM_SIZE);
			memcpy(isp_ini_cfg->isp_tunning_settings.gamma_tbl_post, buf, ISP_GAMMA_MEM_SIZE);
		}
		else
		{
			memcpy(isp_ini_cfg->isp_tunning_settings.gamma_tbl_ini, buf, len);
			memcpy(isp_ini_cfg->isp_tunning_settings.gamma_tbl, buf, len);
			memcpy(isp_ini_cfg->isp_tunning_settings.gamma_tbl_post, buf, len);
		}
	}

	/* fetch lsc table! */
	if(0 == isp_ini_cfg->isp_tunning_settings.lsc_mod){
		len = cfg_read_file(isp_lsc_tbl_path,buf,SIZE_OF_LSC_TBL_MOD0);
	}else{
		len = cfg_read_file(isp_lsc_tbl_path,buf,SIZE_OF_LSC_TBL_MOD1);
	}
	if(len < 0)
	{
		vfe_warn("read lsc_tbl from lsc_tbl.bin failed!\n");
		ret =  -1;
	}
	else
	{
		memcpy(isp_ini_cfg->isp_tunning_settings.lsc_tbl, buf, len);
	}
	/* fetch hdr_tbl table!*/
	if(0 != ISP_LINEAR_MEM_SIZE && 0 != ISP_DISC_MEM_SIZE)
	{
		vfe_print("read disc and linear table!\n");
		len = cfg_read_file(isp_hdr_tbl_path,buf, ISP_LINEAR_MEM_SIZE +ISP_DISC_MEM_SIZE);
		if(len < 0)
		{
			vfe_warn("read hdr_tbl, linear_tbl and disc_tbl from hdr_tbl.bin failed!\n");
			//ret =  -1;
		}
		else
		{
			//memcpy(isp_ini_cfg->isp_tunning_settings.hdr_tbl, buf, 4*ISP_DRC_MEM_SIZE);
			memcpy(isp_ini_cfg->isp_tunning_settings.linear_tbl, buf , ISP_LINEAR_MEM_SIZE);
			memcpy(isp_ini_cfg->isp_tunning_settings.disc_tbl, buf +ISP_LINEAR_MEM_SIZE, ISP_DISC_MEM_SIZE);
		}
	}
	if(buf)
	{
		kfree(buf);
	}
	return ret;
}

int match_isp_cfg(struct vfe_dev *dev,int isp_id)
{
	int ret;
	struct isp_cfg_item isp_cfg_tmp;
	struct isp_init_config *isp_ini_cfg = &dev->isp_gen_set[isp_id].isp_ini_cfg;
	ret = get_isp_cfg(dev->ccm_cfg[isp_id]->isp_cfg_name,&isp_cfg_tmp);
	if(ret < 0)
	{
		return -1;
	}
	isp_ini_cfg->isp_3a_settings = *isp_cfg_tmp.isp_cfg->isp_3a_settings;
	isp_ini_cfg->isp_test_settings = *isp_cfg_tmp.isp_cfg->isp_test_settings;
	isp_ini_cfg->isp_tunning_settings = *isp_cfg_tmp.isp_cfg->isp_tunning_settings;
	isp_ini_cfg->isp_iso_settings = *isp_cfg_tmp.isp_cfg->isp_iso_settings;
	memcpy(isp_ini_cfg->isp_tunning_settings.gamma_tbl, isp_ini_cfg->isp_tunning_settings.gamma_tbl_ini, ISP_GAMMA_MEM_SIZE);
	memcpy(isp_ini_cfg->isp_tunning_settings.gamma_tbl_post, isp_ini_cfg->isp_tunning_settings.gamma_tbl_ini, ISP_GAMMA_MEM_SIZE);
	return 0;
}
int read_ini_info(struct vfe_dev *dev,int isp_id, char *main_path)
{
	int i, ret = 0;
	char isp_cfg_path[128],isp_tbl_path[128],file_name_path[128];
	struct cfg_section *cfg_section;
	struct file* fp;

	if(dev->ccm_cfg[isp_id] != NULL && strcmp(dev->ccm_cfg[isp_id]->isp_cfg_name, "") != 0)
	{
		sprintf(isp_cfg_path, "%s%s/",main_path, dev->ccm_cfg[isp_id]->isp_cfg_name);
		sprintf(isp_tbl_path, "%s%s/bin/", main_path, dev->ccm_cfg[isp_id]->isp_cfg_name);

		//sprintf(isp_cfg_path, "/system/etc/hawkview/%s/", dev->ccm_cfg[isp_id]->isp_cfg_name);
		//sprintf(isp_tbl_path, "/system/etc/hawkview/%s/bin/", dev->ccm_cfg[isp_id]->isp_cfg_name);
		//sprintf(isp_cfg_path, "/mnt/extsd/hawkview/%s/", dev->ccm_cfg[isp_id]->isp_cfg_name);
		//sprintf(isp_tbl_path, "/mnt/extsd/hawkview/%s/bin/", dev->ccm_cfg[isp_id]->isp_cfg_name);
	}
	else
	{
		sprintf(isp_cfg_path, "/system/etc/hawkview/camera.ini");
		sprintf(isp_tbl_path, "/system/etc/hawkview/bin/");
	}

	
	sprintf(file_name_path,"%s%s",isp_cfg_path,FileAttr[0].file_name);	  
	fp = filp_open(isp_cfg_path,O_RDONLY,0);
	if(IS_ERR(fp)) {
		vfe_print("Check open %s failed!\nMatch isp cfg  start!\n", file_name_path);
		if(match_isp_cfg(dev,isp_id) == 0)
		{
			vfe_print("Match isp cfg ok\n");
			goto read_ini_info_end;
		}
	}
	vfe_print("read ini start\n");
	
	dev->isp_gen_set[isp_id].isp_ini_cfg = isp_init_def_cfg;
	for(i=0; i< ARRAY_SIZE(FileAttr); i++)
	{
		sprintf(file_name_path,"%s%s",isp_cfg_path,FileAttr[i].file_name);
		vfe_print("Fetch ini file form \"%s\"\n",file_name_path);
		cfg_section_init(&cfg_section);
		ret = cfg_read_ini(file_name_path, &cfg_section);
		if(ret == -1)
		{
			cfg_section_release(&cfg_section);
			goto read_ini_info_end;
		}
		fetch_isp_cfg(&dev->isp_gen_set[isp_id].isp_ini_cfg, cfg_section,&FileAttr[i]);
		cfg_section_release(&cfg_section);
	}
	ret = fetch_isp_tbl(&dev->isp_gen_set[isp_id].isp_ini_cfg, &isp_tbl_path[0]);
	if(ret == -1)
    	{
    		dev->isp_gen_set[isp_id].isp_ini_cfg = isp_init_def_cfg;
        }
read_ini_info_end:
	vfe_dbg(0,"read ini end\n");
	return ret;
}


