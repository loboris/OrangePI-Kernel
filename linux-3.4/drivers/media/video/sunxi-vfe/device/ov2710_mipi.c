/*
 * A V4L2 driver for OV2710 Raw cameras.
 *
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/videodev2.h>
#include <linux/clk.h>
#include <media/v4l2-device.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-mediabus.h>
#include <linux/io.h>


#include "camera.h"


MODULE_AUTHOR("xiongbiao");
MODULE_DESCRIPTION("A low-level driver for OV2710 Raw sensors");
MODULE_LICENSE("GPL");

//for internel driver debug
#define DEV_DBG_EN      1 
#if(DEV_DBG_EN == 1)    
#define vfe_dev_dbg(x,arg...) printk("[OV2710 Raw]"x,##arg)
#else
#define vfe_dev_dbg(x,arg...) 
#endif
#define vfe_dev_err(x,arg...) printk("[OV2710 Raw]"x,##arg)
#define vfe_dev_print(x,arg...) printk("[OV2710 Raw]"x,##arg)

#define LOG_ERR_RET(x)  { \
                          int ret;  \
                          ret = x; \
                          if(ret < 0) {\
                            vfe_dev_err("error at %s\n",__func__);  \
                            return ret; \
                          } \
                        }

//define module timing
#define MCLK              (24*1000*1000)
#define VREF_POL          V4L2_MBUS_VSYNC_ACTIVE_LOW
#define HREF_POL          V4L2_MBUS_HSYNC_ACTIVE_HIGH
#define CLK_POL           V4L2_MBUS_PCLK_SAMPLE_RISING
#define V4L2_IDENT_SENSOR  0x2710


//define the voltage level of control signal
#define CSI_STBY_ON     1
#define CSI_STBY_OFF    0
#define CSI_RST_ON      0
#define CSI_RST_OFF     1
#define CSI_PWR_ON      1
#define CSI_PWR_OFF     0
#define CSI_AF_PWR_ON   1
#define CSI_AF_PWR_OFF  0
#define regval_list reg_list_a16_d8


#define REG_TERM 0xfffe
#define VAL_TERM 0xfe
#define REG_DLY  0xffff

/*
 * Our nominal (default) frame rate.
 */

#define SENSOR_FRAME_RATE 30


/*
 * The ov2710 i2c address
 */
//#define I2C_ADDR 0x6c
#define OV2710_WRITE_ADDR (0x6c)
#define OV2710_READ_ADDR  (0x6d)

//static struct delayed_work sensor_s_ae_ratio_work;
static struct v4l2_subdev *glb_sd;
#define SENSOR_NAME "ov2710_mipi"

/*
 * Information we maintain about a known sensor.
 */
struct sensor_format_struct;  /* coming later */

struct cfg_array { /* coming later */
	struct regval_list * regs;
	int size;
};

static inline struct sensor_info *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct sensor_info, sd);
}


/*
 * The default register settings
 *
 */

static struct regval_list sensor_default_regs[] = 
{};

static struct regval_list sensor_1080p_regs[] = { //1080: 1920*1080@30fps
	{0x3103,0x93},
	{0x3008,0x82},
	{REG_DLY,0x10},
	{0x3008,0x42},
	{0x3017,0x00},//7f},pad output enable01  0-input 1-output
	{0x3018,0x00},//fc},pad output enable02  0-input 1-output
	{0x3706,0x61},
	{0x3712,0x0c},
	{0x3630,0x6d},
	{0x3801,0xb4},
	{0x3621,0x04},
	{0x3604,0x60},
	{0x3603,0xa7},
	{0x3631,0x26},
	{0x3600,0x04},
	{0x3620,0x37},
	{0x3623,0x00},
	{0x3702,0x9e},
	{0x3703,0x5c},
	{0x3704,0x40},
	{0x370d,0x0f},
	{0x3713,0x9f},
	{0x3714,0x4c},
	{0x3710,0x9e},
	{0x3801,0xc4},
	{0x3605,0x05},
	{0x3606,0x3f},
	{0x302d,0x90},
	{0x370b,0x40},
	{0x3716,0x31},
	{0x3707,0x52},
	{0x380d,0x74},
	{0x5181,0x20},
	{0x518f,0x00},
	{0x4301,0xff},
	{0x4303,0x00},
	{0x3a00,0x78},
//	{0x300f,0x88},
//	{0x3011,0x28},
	{0x3a1a,0x06},
	{0x3a18,0x00},
	{0x3a19,0x7a},
	{0x3a13,0x54},
	{0x382e,0x0f},
	{0x381a,0x1a},
	{0x401d,0x02},
	{0x5688,0x03},
	{0x5684,0x07},
	{0x5685,0xa0},
	{0x5686,0x04},
	{0x5687,0x43},
	{0x3011,0x0a},
	{0x300f,0xc3},
	{REG_DLY,0x10},
	//{0x3017,0x00},
	//{0x3018,0x00},
	{0x300e,0x04},
	{0x3030,0x2b},// extent dvdd 
	{0x4801,0x0f},
//	{0x4800,0x24},
	//AEC control 
	{0x3a0f,0x40},
	{0x3a10,0x38},
	{0x3a1b,0x48},
	{0x3a1e,0x30},
	{0x3a11,0x90},
	{0x3a1f,0x10},
	//{0x3008,0x02},


	/*close ae */
	{0x3503,0x07},
	//{0x302c,0x02},
	{0x3501,0x2e},
	{0x3502,0x00},
	//{0x3501,0x46},
	//{0x3502,0x20},
	{0x350b,0x10},
	//{0x3a00,0x7c},
	//{0x5001,0x4f},
	
	/* close awb */  
    {0x5001,0x4f},
	{0x3406,0x01},
	{0x3400,0x04},
	{0x3401,0x00},
	{0x3402,0x04},
	{0x3403,0x00},
	{0x3404,0x04},
	{0x3405,0x00},

	/* modified blc */
	//{0x4002,0x40},
	{0x4000,0x05},
	//{0x401d,0x02},
	/*drive capacity*/
	{0x302c,0x00},
	/* close lenc */
	//{0x5000,0xdf},
	{0x5000,0x5b},
	{0x3008,0x02},//nomal work mode
};


#if 0
static struct regval_list sensor_720p_regs[] = { //720: 1280*720@60fps
	{0x3103,0x93},
	{0x3008,0x82},//system control 
	{0x3017,0x00},//7f},pad output enable01  0-input 1-output
	{0x3018,0x00},//fc},pad output enable02  0-input 1-output
	{0x3706,0x61},
	{0x3712,0x0c},
	{0x3630,0x6d},
	{0x3801,0xb4},
	{0x3621,0x04},
	{0x3604,0x60},
	{0x3603,0xa7},
	{0x3631,0x26},
	{0x3600,0x04},
	{0x3620,0x37},
	{0x3623,0x00},
	{0x3702,0x9e},
	{0x3703,0x5c},
	{0x3704,0x40},
	{0x370d,0x0f},
	{0x3713,0x9f},
	{0x3714,0x4c},
	{0x3710,0x9e},
	{0x3801,0xc4},
	{0x3605,0x05},
	{0x3606,0x3f},
	{0x302d,0x90},
	{0x370b,0x40},
	{0x3716,0x31},
	{0x3707,0x52},
	{0x380d,0x74},
	{0x5181,0x20},
	{0x518f,0x00},
	{0x4301,0xff},
	{0x4303,0x00},
	//turn of night mode 3a00[2]->0
	{0x3a00,0x78},
	{0x300f,0x88},
	{0x3011,0x28},
	{0x3a1a,0x06},
	{0x3a18,0x00},
	{0x3a19,0x7a},
	{0x3a13,0x54},
	{0x382e,0x0f},
	{0x381a,0x1a},
	{0x401d,0x02},
	{0x381c,0x10},
	{0x381d,0xb8},
	{0x381e,0x02},
	{0x381f,0xdc},
	{0x3820,0x0a},
	{0x3821,0x29},
	{0x3804,0x05},
	{0x3805,0x00},
	{0x3806,0x02},
	{0x3807,0xd0},
	{0x3808,0x05},
	{0x3809,0x00},
	{0x380a,0x02},
	{0x380b,0xd0},
	{0x380e,0x02},
	{0x380f,0xe8},
	{0x380c,0x07},
	{0x380d,0x00},
	{0x5688,0x03},
	{0x5684,0x05},
	{0x5685,0x00},
	{0x5686,0x02},
	{0x5687,0xd0},
	//50hz band	
	{0x3a08,0x1b},
	{0x3a09,0xe6},
	//60hz band
	{0x3a0a,0x17},
	{0x3a0b,0x40},
	{0x3a0e,0x01},
	{0x3a0d,0x02},
	{0x3011,0x0a},
	{0x300f,0xc3},
	//{0x3017,0x00},
	//{0x3018,0x00},

	{0x300e,0x04},
	{0x4801,0x0f},
	{0x4800,0x24},
	{0x3a0f,0x40},
	{0x3a10,0x38},
	{0x3a1b,0x48},
	{0x3a1e,0x30},
	{0x3a11,0x90},
	{0x3a1f,0x10},
	//{0x3008,0xc2},
};
#endif

//static struct regval_list sensor_svga_regs[] = { //SVGA: 800*600
//
//};

//static struct regval_list sensor_vga_regs[] = { //VGA:  640*480
//};


/*
 * Here we'll try to encapsulate the changes for just the output
 * video format.
 * 
 */

static struct regval_list sensor_fmt_raw[] = {
//{REG_TERM,VAL_TERM},
};

/*
 * Low-level register I/O.
 *
 */


/*
 * On most platforms, we'd rather do straight i2c I/O.
 */

static int sensor_read(struct v4l2_subdev *sd, unsigned short reg,
    unsigned char *value)
{
	int ret=0;
	int cnt=0;
	
	ret = cci_read_a16_d8(sd,reg,value);
	while(ret!=0&&cnt<3)
	{
		ret = cci_read_a16_d8(sd,reg,value);
		cnt++;
	}
	if(cnt>0)
		vfe_dev_dbg("sensor read retry=%d\n",cnt);
  
	return ret;
}

static int reg_val_show(struct v4l2_subdev *sd,unsigned short reg)
{
	unsigned char tmp;
	sensor_read(sd,reg,&tmp);
	printk("0x%x value is 0x%x\n",reg,tmp);
	return 0;
}

/*static int show_regs_array (struct v4l2_subdev *sd,struct regval_list *array)
{	int i =0;
	unsigned char tmp;
	
	for(i=0;i<(sizeof(sensor_1080p_regs)/3);i++) 
	{		if((array+i)->addr == REG_DLY) 
		 {
               msleep((array+i)->data);
   	     } 
		sensor_read(sd,(array+i)->addr,&tmp);
		printk("ov2710 0x%x value is 0x%x\n",(array+i)->addr,tmp);
	}
	return 0;
}
*/

static int sensor_write(struct v4l2_subdev *sd, unsigned short reg,
    unsigned char value)
{
	int ret=0;
	int cnt=0;
	ret = cci_write_a16_d8(sd,reg,value);
	while(ret!=0&&cnt<2)
	{
		ret = cci_write_a16_d8(sd,reg,value);
		cnt++;
	}
	if(cnt>0)
		vfe_dev_dbg("sensor write retry=%d\n",cnt);

	return ret;
}

/*
 * Write a list of register settings;
 */
static int sensor_write_array(struct v4l2_subdev *sd, struct regval_list *regs, int array_size)
{
	int i=0;
	
	if(!regs)
	return -EINVAL;

	while(i<array_size)
	{
		if(regs->addr == REG_DLY) {
			msleep(regs->data);
		} else {
			LOG_ERR_RET(sensor_write(sd, regs->addr, regs->data))
		}
		i++;
		regs++;
	}
	return 0;
}

/* 
 * Code for dealing with controls.
 * fill with different sensor module
 * different sensor module has different settings here
 * if not support the follow function ,retrun -EINVAL
 */
static int sensor_g_exp(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	
	*value = info->exp;
	vfe_dev_dbg("sensor_get_exposure = %d\n", info->exp);
	return 0;
}

static int sensor_s_exp(struct v4l2_subdev *sd, unsigned int exp_val)
{
	unsigned char explow,expmid,exphigh;
	struct sensor_info *info = to_state(sd);
	
	if(exp_val>0x1fffff)
		exp_val=0x1fffff;
	
	exphigh = (unsigned char) ( (0x0f0000&exp_val)>>16);
	expmid  = (unsigned char) ( (0x00ff00&exp_val)>>8);
	explow  = (unsigned char) (0x0000ff&exp_val) ;
	
	sensor_write(sd, 0x3502, explow);
	sensor_write(sd, 0x3501, expmid);
	sensor_write(sd, 0x3500, exphigh);
//	printk("2710 sensor_set_exp = %d Done!\n", exp_val );

	info->exp = exp_val;
	return 0;
}


static int sensor_g_gain(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);
	
	*value = info->gain;
	vfe_dev_dbg("sensor_get_gain = %d\n", info->gain);
	return 0;
}

static int sensor_s_gain(struct v4l2_subdev *sd, int gain_val)
{		
	struct sensor_info *info = to_state(sd);
	unsigned char gainlow=0;
	unsigned char gainhigh=0;
	unsigned char gainlow_l4b=0;
	unsigned int tmp_gain_val=0,real_gain = 1, awb_gain = 0;
	
	tmp_gain_val=gain_val;
	
	//determine ?gain_val>31
	if(tmp_gain_val>31)
	{
		gainlow |= 0x10;
		real_gain = real_gain * 2;
		tmp_gain_val = tmp_gain_val>>1;
	}
	//determine ?gain_val>2*31
	if(tmp_gain_val>31)
	{
		gainlow |= 0x20;
		real_gain = real_gain * 2;
		tmp_gain_val = tmp_gain_val>>1;
	}
	//determine ?gain_val>4*31
	if(tmp_gain_val>31)
	{
		gainlow |= 0x40;
		real_gain = real_gain * 2;
		tmp_gain_val = tmp_gain_val>>1;
	}
	//determine ?gain_val>8*31
	if(tmp_gain_val>31)
	{
		gainlow |= 0x80;
		real_gain = real_gain * 2;
		tmp_gain_val = tmp_gain_val>>1;
	}
	//determine ?gain_val>16*31
	if(tmp_gain_val>31)
	{
		gainhigh = 0x01;
		real_gain = real_gain * 2;
		tmp_gain_val = tmp_gain_val>>1;
	}
	if(tmp_gain_val>=16)
		gainlow_l4b=((tmp_gain_val-16)&0x0f);
	gainlow  = gainlow | gainlow_l4b;
    
    real_gain = real_gain * (16*gainlow_l4b /16 +16);
    awb_gain = gain_val * 1024/real_gain;
//	if(info->gain == gain_val)
//		return 0;
	sensor_write(sd, 0x350b, gainlow);
	sensor_write(sd, 0x350a, gainhigh);
    
	sensor_write(sd, 0x3400, (awb_gain >> 8));
	sensor_write(sd, 0x3401, (awb_gain & 0xff));
	sensor_write(sd, 0x3402, (awb_gain >> 8));
	sensor_write(sd, 0x3403, (awb_gain & 0xff));
	sensor_write(sd, 0x3404, (awb_gain >> 8));
	sensor_write(sd, 0x3405, (awb_gain & 0xff));
//	printk("2710 sensor_set_gain = %d, 0x%x, 0x%x Done!\n", gain_val,gainlow, gainhigh );
	info->gain = gain_val;
	
	return 0;
	
}

static int ov2710_sensor_vts;
static int sensor_s_exp_gain(struct v4l2_subdev *sd, struct sensor_exp_gain *exp_gain)
{
  int exp_val, gain_val,shutter,frame_length;  
//  unsigned char explow=0,expmid=0,exphigh=0,vts_diff_low,vts_diff_high;
//  unsigned char gainlow=0,gainhigh=0;  
  struct sensor_info *info = to_state(sd);

  exp_val = exp_gain->exp_val;
  gain_val = exp_gain->gain_val;

  //if((info->exp == exp_val)&&(info->gain == gain_val))
  //	return 0;
  if(gain_val<1*16)
	  gain_val=16;
  if(gain_val>64*16-1)
	  gain_val=64*16-1;
  
  if(exp_val>0xfffff)
	  exp_val=0xfffff;

  
  shutter = exp_val/16;
  if(shutter > ov2710_sensor_vts- 4)
		frame_length = shutter + 4;
  else
		frame_length = ov2710_sensor_vts;

  sensor_write(sd,0x3212,0x00);
  sensor_write(sd, 0x380f, (frame_length & 0xff));
  sensor_write(sd, 0x380e, (frame_length >> 8));
  sensor_s_exp(sd,exp_val);
  sensor_s_gain(sd,gain_val);
  sensor_write(sd,0x3212,0x10);
  sensor_write(sd,0x3212,0xa0);
  
 // printk("2710 sensor_set_gain gain= %d,exp= %d Done!\n", gain_val,exp_val);

  info->exp = exp_val;
  info->gain = gain_val;
  return 0;
}


static int sensor_s_sw_stby(struct v4l2_subdev *sd, int on_off)
{
	int ret = 0;
	return ret;
}

/*
 * Stuff that knows about the sensor.
 */
 
static int sensor_power(struct v4l2_subdev *sd, int on)
{
	//  struct i2c_client *client = v4l2_get_subdevdata(sd);
	int ret;

	//insure that clk_disable() and clk_enable() are called in pair 
	//when calling CSI_SUBDEV_STBY_ON/OFF and CSI_SUBDEV_PWR_ON/OFF
	ret = 0;
	switch(on)
	{
		case CSI_SUBDEV_STBY_ON:
			vfe_dev_dbg("CSI_SUBDEV_STBY_ON!\n");
			//software standby on
			ret = sensor_s_sw_stby(sd, CSI_GPIO_HIGH);
			if(ret < 0)
				vfe_dev_err("soft stby falied!\n");
			usleep_range(10000,12000);
			//when using i2c_lock_adpater function, the following codes must not access i2c bus before calling i2c_unlock_adapter
			cci_lock(sd);
			//standby on io
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			//remember to unlock i2c adapter, so the device can access the i2c bus again
			cci_unlock(sd);
			//inactive mclk after stadby in
			vfe_set_mclk(sd,OFF);
			break;
		case CSI_SUBDEV_STBY_OFF:
			vfe_dev_dbg("CSI_SUBDEV_STBY_OFF!\n");
			cci_lock(sd);
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(10000,12000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW);
			usleep_range(10000,12000);
			ret = sensor_s_sw_stby(sd, CSI_GPIO_LOW);
			if(ret < 0)
				vfe_dev_err("soft stby off falied!\n");
			cci_unlock(sd);
			break;
		case CSI_SUBDEV_PWR_ON:
			vfe_dev_dbg("CSI_SUBDEV_PWR_ON!\n");
			cci_lock(sd);
			//power on reset
			vfe_gpio_set_status(sd,PWDN,1);//set the gpio to output
			vfe_gpio_set_status(sd,RESET,1);//set the gpio to output
			vfe_gpio_set_status(sd,POWER_EN,1);//set the gpio to output 
			
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);  
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_LOW);
			usleep_range(1000,1200);
			//power supply
			vfe_set_pmu_channel(sd,IOVDD,ON);
			//add by chao 
			usleep_range(1000,1200);
			vfe_set_pmu_channel(sd,AVDD,ON);	
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_HIGH);
			vfe_set_pmu_channel(sd,DVDD,ON);
			vfe_set_pmu_channel(sd,AFVDD,ON);
			//add by chao
			usleep_range(7000,8000);
			vfe_gpio_write(sd,PWDN,CSI_GPIO_LOW); 
			usleep_range(10000,12000); 
			//reset on io
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);
			usleep_range(20000,22000);
			vfe_gpio_write(sd,RESET,CSI_GPIO_HIGH);
			//active mclk before power on
			vfe_set_mclk_freq(sd,MCLK);
			vfe_set_mclk(sd,ON);
			usleep_range(10000,12000);
			cci_unlock(sd);
			break;
		case CSI_SUBDEV_PWR_OFF:
			vfe_dev_dbg("CSI_SUBDEV_PWR_OFF!\n");
			cci_lock(sd);
			vfe_gpio_set_status(sd,PWDN,1);//set the gpio to output
			vfe_gpio_set_status(sd,RESET,1);//set the gpio to output
			vfe_gpio_write(sd,RESET,CSI_GPIO_LOW);  
			vfe_gpio_write(sd,PWDN,CSI_GPIO_HIGH);
			//inactive mclk before power off
			vfe_set_mclk(sd,OFF);
			//power supply off
			vfe_set_pmu_channel(sd,AFVDD,OFF);
			vfe_set_pmu_channel(sd,DVDD,OFF);
			vfe_gpio_write(sd,POWER_EN,CSI_GPIO_LOW);
			vfe_set_pmu_channel(sd,AVDD,OFF);
			vfe_set_pmu_channel(sd,IOVDD,OFF);
			//set the io to hi-z
			vfe_gpio_set_status(sd,RESET,0);//set the gpio to input
			vfe_gpio_set_status(sd,PWDN,0);//set the gpio to input
			vfe_gpio_set_status(sd,POWER_EN,0);//set the gpio to input
			cci_unlock(sd);
			break;
		default:
			return -EINVAL;
	}

	return 0;
}
 
static int sensor_reset(struct v4l2_subdev *sd, u32 val)
{
	switch(val)
	{
		case 0:
			vfe_gpio_write(sd,RESET,CSI_RST_OFF);
			usleep_range(10000,12000);
			break;
		case 1:
			vfe_gpio_write(sd,RESET,CSI_RST_ON);
			usleep_range(10000,12000);
			break;
		default:
			return -EINVAL;
	}
	return 0;
}

static int sensor_detect(struct v4l2_subdev *sd)
{
	unsigned char rdval = 0;
	vfe_dev_dbg("sensor_detect_1--!\n");
  
	LOG_ERR_RET(sensor_read(sd, 0x300A, &rdval));
	vfe_dev_dbg("sensor_detect_2--!\n");

	if(rdval != (V4L2_IDENT_SENSOR>>8))
	{
		printk(KERN_DEBUG"*********sensor error,read id is %d.\n",rdval);
		return -ENODEV;
	}

	LOG_ERR_RET(sensor_read(sd, 0x300B, &rdval))
	if(rdval != (V4L2_IDENT_SENSOR&0x00ff))
	{
		printk(KERN_DEBUG"*********sensor error,read id is %d.\n",rdval);
		return -ENODEV;
	}
	else
	{
		printk(KERN_DEBUG"*********find ov2710 raw data camera sensor now.\n");
		return 0;
	}
}

static int sensor_init(struct v4l2_subdev *sd, u32 val)
{
	int ret;
	struct sensor_info *info = to_state(sd);
   
	vfe_dev_dbg("sensor_init\n");
  
	/*Make sure it is a target sensor*/
	ret = sensor_detect(sd);
	if (ret) {
		vfe_dev_err("chip found is not an target chip.\n");
		return ret;
	}
  
	vfe_get_standby_mode(sd,&info->stby_mode);

	if((info->stby_mode == HW_STBY || info->stby_mode == SW_STBY) \
	&& info->init_first_flag == 0) {
		vfe_dev_print("stby_mode and init_first_flag = 0\n");
		return 0;
	} 
  
	info->focus_status = 0;
	info->low_speed = 0;
	info->width = HD1080_WIDTH;
	info->height = HD1080_HEIGHT;
	info->hflip = 0;
	info->vflip = 0;
	info->gain = 0;

	info->tpf.numerator = 1;            
	info->tpf.denominator = 30;    /* 30fps */    

	ret = sensor_write_array(sd, sensor_default_regs, ARRAY_SIZE(sensor_default_regs));  
	if(ret < 0) {
		vfe_dev_err("write sensor_default_regs error\n");
		return ret;
	}
  
	if(info->stby_mode == 0)
		info->init_first_flag = 0;
  
	info->preview_first_flag = 1;
     
	reg_val_show(sd,0x3012);
	
	reg_val_show(sd,0x380c);
  
	return 0;
}

static long sensor_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret=0;
	struct sensor_info *info = to_state(sd);

	switch(cmd) {
	case GET_CURRENT_WIN_CFG:
		if(info->current_wins != NULL)
		{
			memcpy( arg,
			        info->current_wins,
			        sizeof(struct sensor_win_size) );
			ret=0;
		}
		else
		{
			vfe_dev_err("empty wins!\n");
			ret=-1;
		}
		break;
	case SET_FPS:
		ret=0;
		break;
	case ISP_SET_EXP_GAIN:
		ret = sensor_s_exp_gain(sd, (struct sensor_exp_gain *)arg);
		break;
	default:
		return -EINVAL;
	}
	return ret;
}


/*
 * Store information about the video data format. 
 */
static struct sensor_format_struct {
  __u8 *desc;
  //__u32 pixelformat;
  enum v4l2_mbus_pixelcode mbus_code;
  struct regval_list *regs;
  int regs_size;
  int bpp;   /* Bytes per pixel */
}sensor_formats[] = {
	{
		.desc		= "Raw RGB Bayer",
		.mbus_code	=V4L2_MBUS_FMT_SBGGR10_10X1,
		.regs 		= sensor_fmt_raw,
		.regs_size = ARRAY_SIZE(sensor_fmt_raw),
		.bpp		= 1
	},
};
#define N_FMTS ARRAY_SIZE(sensor_formats)

/*
 * Then there is the issue of window sizes.  Try to capture the info here.
 */

static struct sensor_win_size sensor_win_sizes[] = {
	/* 1080P */
	{
		.width	= HD1080_WIDTH,
		.height 	= HD1080_HEIGHT,
		.hoffset	  = 0,
		.voffset	  = 0,
		.hts        = 2420,//2376,//2415,// 2382,//724,
		.vts        = 1102,//1122,//1104,//1120,//1104,
		.pclk       = 80*1000*1000,
		.mipi_bps		= 800*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1,
		.intg_max   = 1102<<4,//1122<<4,
		.gain_min   = 1<<4,
		.gain_max   = 16<<4,
		.regs       = sensor_1080p_regs,//
		.regs_size  = ARRAY_SIZE(sensor_1080p_regs),//
		.set_size		= NULL,
	},

	/* 720P */
	{
		.width	= HD720_WIDTH,
		.height 	= HD720_HEIGHT,
		.hoffset	  = 0,
		.voffset	  = 0,
		.hts		= 2420,//2376,//2415,// 2382,//724,
		.vts		= 1102,//1122,//1104,//1120,//1104,
		.pclk		= 80*1000*1000,
		.mipi_bps		= 800*1000*1000,
		.fps_fixed	= 1,
		.bin_factor = 1,
		.intg_min	= 1,
		.intg_max	= 1102<<4,//1122<<4,
		.gain_min	= 1<<4,
		.gain_max	= 16<<4,
		.width_input	  = 1920,
		.height_input	  = 1080,	
		.regs		= sensor_1080p_regs,//
		.regs_size	= ARRAY_SIZE(sensor_1080p_regs),//
		.set_size		= NULL,
	},

	/* VGA */
	{
		.width	  = VGA_WIDTH,
		.height 	  = VGA_HEIGHT,
		.hoffset	  = 240,
		.voffset	  = 0,
		.hts		  = 2420,//2376,//2415,// 2382,//724,
		.vts		  = 1102,//1122,//1104,//1120,//1104,
		.pclk 	  = 80*1000*1000,
		.mipi_bps 	= 800*1000*1000,
		.fps_fixed  = 1,
		.bin_factor = 1,
		.intg_min   = 1,
		.intg_max   = 1102<<4,//1122<<4,
		.gain_min   = 1<<4,
		.gain_max   = 16<<4,
		.width_input	  = 1440,
		.height_input	  = 1080,	
		.regs 	  = sensor_1080p_regs,//
		.regs_size  = ARRAY_SIZE(sensor_1080p_regs),//
		.set_size 	= NULL,
	},
};

#define N_WIN_SIZES (ARRAY_SIZE(sensor_win_sizes))

static int sensor_enum_fmt(struct v4l2_subdev *sd, unsigned index,
                 enum v4l2_mbus_pixelcode *code)
{
	if (index >= N_FMTS)
		return -EINVAL;

	*code = sensor_formats[index].mbus_code;
	return 0;
}

static int sensor_enum_size(struct v4l2_subdev *sd,
                            struct v4l2_frmsizeenum *fsize)
{
	if(fsize->index > N_WIN_SIZES-1)
		return -EINVAL;
  
	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->discrete.width = sensor_win_sizes[fsize->index].width;
	fsize->discrete.height = sensor_win_sizes[fsize->index].height;
  
	return 0;
}

static int sensor_try_fmt_internal(struct v4l2_subdev *sd,
    struct v4l2_mbus_framefmt *fmt,
    struct sensor_format_struct **ret_fmt,
    struct sensor_win_size **ret_wsize)
{
	int index;
	struct sensor_win_size *wsize;
	struct sensor_info *info = to_state(sd);

	for (index = 0; index < N_FMTS; index++)
		if (sensor_formats[index].mbus_code == fmt->code)
		break;

	if (index >= N_FMTS) 
		return -EINVAL;

	if (ret_fmt != NULL)
		*ret_fmt = sensor_formats + index;
    
	/*
	* Fields: the sensor devices claim to be progressive.
	*/

	fmt->field = V4L2_FIELD_NONE;

	/*
	* Round requested image size down to the nearest
	* we support, but not below the smallest.
	*/
	for (wsize = sensor_win_sizes; wsize < sensor_win_sizes + N_WIN_SIZES;
	   wsize++)
		if (fmt->width >= wsize->width && fmt->height >= wsize->height)
			break;
    
	if (wsize >= sensor_win_sizes + N_WIN_SIZES)
		wsize--;   /* Take the smallest one */
	if (ret_wsize != NULL)
		*ret_wsize = wsize;
	/*
	* Note the size we'll actually handle.
	*/
	fmt->width = wsize->width;
	fmt->height = wsize->height;
	info->current_wins = wsize;
	//pix->bytesperline = pix->width*sensor_formats[index].bpp;
	//pix->sizeimage = pix->height*pix->bytesperline;

	return 0;
}

static int sensor_try_fmt(struct v4l2_subdev *sd, 
             struct v4l2_mbus_framefmt *fmt)
{
	return sensor_try_fmt_internal(sd, fmt, NULL, NULL);
}

static int sensor_g_mbus_config(struct v4l2_subdev *sd,
           struct v4l2_mbus_config *cfg)
{
	//cfg->type = V4L2_MBUS_PARALLEL;
	//cfg->flags = V4L2_MBUS_MASTER | VREF_POL | HREF_POL | CLK_POL ;
	cfg->type = V4L2_MBUS_CSI2;
	cfg->flags = 0|V4L2_MBUS_CSI2_1_LANE|V4L2_MBUS_CSI2_CHANNEL_0;
	return 0;
}


/*
 * Set a format.
 */
static int sensor_s_fmt(struct v4l2_subdev *sd, 
             struct v4l2_mbus_framefmt *fmt)
{
	int ret;
	struct sensor_format_struct *sensor_fmt;
	struct sensor_win_size *wsize;
	struct sensor_info *info = to_state(sd);

	vfe_dev_dbg("sensor_s_fmt\n");

	//  sensor_write_array(sd, sensor_oe_disable_regs, ARRAY_SIZE(sensor_oe_disable_regs));
  
	ret = sensor_try_fmt_internal(sd, fmt, &sensor_fmt, &wsize);
	if (ret)
	return ret;
  
	if(info->capture_mode == V4L2_MODE_VIDEO)
	{
	//video
	}
	else if(info->capture_mode == V4L2_MODE_IMAGE)
	{
	//image 
	}
  
	LOG_ERR_RET(sensor_write_array(sd, sensor_fmt->regs, sensor_fmt->regs_size))
  
	ret = 0;
	if (wsize->regs)
	{
		// usleep_range(5000,6000);
		LOG_ERR_RET(sensor_write_array(sd, wsize->regs, wsize->regs_size))
	}

	if (wsize->set_size)
		LOG_ERR_RET(wsize->set_size(sd))

	info->fmt = sensor_fmt;
	info->width = wsize->width;
	info->height = wsize->height;
	info->exp = 0;
	info->gain = 0;
	ov2710_sensor_vts = wsize->vts;  
   
	vfe_dev_print("s_fmt set width = %d, height = %d\n",wsize->width,wsize->height);

	if(info->capture_mode == V4L2_MODE_VIDEO)
	{
		//video
	} else {
		//capture image
	}
	//sensor_write_array(sd, sensor_oe_enable_regs, ARRAY_SIZE(sensor_oe_enable_regs));
	
	return 0;
}

/*
 * Implement G/S_PARM.  There is a "high quality" mode we could try
 * to do someday; for now, we just do the frame rate tweak.
 */
static int sensor_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
	struct v4l2_captureparm *cp = &parms->parm.capture;
	struct sensor_info *info = to_state(sd);

	if (parms->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;
  
	memset(cp, 0, sizeof(struct v4l2_captureparm));
	cp->capability = V4L2_CAP_TIMEPERFRAME;
	cp->capturemode = info->capture_mode;
     
	return 0;
}

static int sensor_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *parms)
{
	struct v4l2_captureparm *cp = &parms->parm.capture;
	//struct v4l2_fract *tpf = &cp->timeperframe;
	struct sensor_info *info = to_state(sd);
	//unsigned char div;
  
	vfe_dev_dbg("sensor_s_parm\n");
  
	if (parms->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;
  
	if (info->tpf.numerator == 0)
		return -EINVAL;
    
	info->capture_mode = cp->capturemode;
  
	return 0;
}


static int sensor_queryctrl(struct v4l2_subdev *sd,
    struct v4l2_queryctrl *qc)
{
	/* Fill in min, max, step and default value for these controls. */
	/* see include/linux/videodev2.h for details */
  
	switch (qc->id) {
	case V4L2_CID_GAIN:
		return v4l2_ctrl_query_fill(qc, 1*16, 16*16, 1, 16);
	case V4L2_CID_EXPOSURE:
		return v4l2_ctrl_query_fill(qc, 1, 65536*16, 1, 1);
	case V4L2_CID_FRAME_RATE:
		return v4l2_ctrl_query_fill(qc, 15, 120, 1, 30);
	}
	return -EINVAL;
}

static int sensor_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	switch (ctrl->id) {
	case V4L2_CID_GAIN:
		return sensor_g_gain(sd, &ctrl->value);
	case V4L2_CID_EXPOSURE:
		return sensor_g_exp(sd, &ctrl->value);
	}
	return -EINVAL;
}

static int sensor_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct v4l2_queryctrl qc;
	int ret;
  
	qc.id = ctrl->id;
	ret = sensor_queryctrl(sd, &qc);
	if (ret < 0) {
		return ret;
	}

	// if (ctrl->value < qc.minimum || ctrl->value > qc.maximum) {
	//   return -ERANGE;
	// }
  
	switch (ctrl->id) {
	case V4L2_CID_GAIN:
		  return sensor_s_gain(sd, ctrl->value);
		  
	case V4L2_CID_EXPOSURE:
		return sensor_s_exp(sd, ctrl->value);
	  
	//case V4L2_CID_FRAME_RATE:
	//  return sensor_s_framerate(sd, ctrl->value);
	}
	return -EINVAL;
}


static int sensor_g_chip_ident(struct v4l2_subdev *sd,
    struct v4l2_dbg_chip_ident *chip)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	return v4l2_chip_ident_i2c_client(client, chip, V4L2_IDENT_SENSOR, 0);
}


/* ----------------------------------------------------------------------- */

static const struct v4l2_subdev_core_ops sensor_core_ops = {
	.g_chip_ident = sensor_g_chip_ident,
	.g_ctrl = sensor_g_ctrl,
	.s_ctrl = sensor_s_ctrl,
	.queryctrl = sensor_queryctrl,
	.reset = sensor_reset,
	.init = sensor_init,
	.s_power = sensor_power,
	.ioctl = sensor_ioctl,
};

static const struct v4l2_subdev_video_ops sensor_video_ops = {
	.enum_mbus_fmt = sensor_enum_fmt,
	.enum_framesizes = sensor_enum_size,
	.try_mbus_fmt = sensor_try_fmt,
	.s_mbus_fmt = sensor_s_fmt,
	.s_parm = sensor_s_parm,
	.g_parm = sensor_g_parm,
	.g_mbus_config = sensor_g_mbus_config,
};

static const struct v4l2_subdev_ops sensor_ops = {
	.core = &sensor_core_ops,
	.video = &sensor_video_ops,
};

/* ----------------------------------------------------------------------- */
static struct cci_driver cci_drv = {
	.name = SENSOR_NAME,
};

static int sensor_probe(struct i2c_client *client,
      const struct i2c_device_id *id)
{
	struct v4l2_subdev *sd;
	struct sensor_info *info;
	//  int ret;

	info = kzalloc(sizeof(struct sensor_info), GFP_KERNEL);
	if (info == NULL)
		return -ENOMEM;
	sd = &info->sd;
	glb_sd = sd;
	cci_dev_probe_helper(sd, client, &sensor_ops, &cci_drv);

	info->fmt = &sensor_formats[0];
	info->af_first_flag = 1;
	info->init_first_flag = 1;

	return 0;
}

static int sensor_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd;
	sd = cci_dev_remove_helper(client, &cci_drv);
	kfree(to_state(sd));
	return 0;
}

static const struct i2c_device_id sensor_id[] = {
  { SENSOR_NAME, 0 },
  { }
};
MODULE_DEVICE_TABLE(i2c, sensor_id);


static struct i2c_driver sensor_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = SENSOR_NAME,
	},
	.probe = sensor_probe,
	.remove = sensor_remove,
	.id_table = sensor_id,
};
static __init int init_sensor(void)
{
	return cci_dev_init_helper(&sensor_driver);
}

static __exit void exit_sensor(void)
{
	cci_dev_exit_helper(&sensor_driver);
}

module_init(init_sensor);
module_exit(exit_sensor);

