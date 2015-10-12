#if 0
#include <asm/uaccess.h>
#include <asm/memory.h>
#include <asm/unistd.h>
#include "asm-generic/int-ll64.h"
#include "linux/kernel.h"
#include "linux/mm.h"
#include "linux/semaphore.h"
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>   //wake_up_process()
#include <linux/kthread.h> //kthread_create()??ékthread_run()
#include <linux/err.h> //IS_ERR()??éPTR_ERR()
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/cdev.h>
#include <linux/types.h>

#if defined(CONFIG_SUNXI_I2C)
#include <linux/i2c.h>
#endif

#include <linux/kernel.h>
#include <linux/init.h>
#include <mach/sys_config.h>
#include <mach/platform.h>
#include <linux/drv_display.h>
#include "mach/ar100.h"
#else
#include <common.h>
#include <malloc.h>
#include <asm/arch/drv_display.h>
#include <asm/arch/intc.h>
#include <asm/arch/cpu.h>
#include <pmu.h>
#include <asm/arch/timer.h>
#include <asm/arch/pwm.h>
#include <i2c.h>
#include <axp_power.h>
#include "../de/bsp_display.h"

#endif

#define GM7121_Config(sub_addr,data) tv_i2c_write(sub_addr, data)

static disp_tv_mode g_tv_mode = DISP_TV_MOD_PAL;
static u32 tv_used = 0;
static u32 tv_i2c_id = 0;
static u32 tv_i2c_used = 0;
//static u32 tv_screen_id = 0;
//static char modules_name[64] = "gm7121";
/* 0x46为I2C设备地址 */
static unsigned short normal_i2c[] = {0x46}; 
static u32 tv_power_used = 0;
static char tv_power[16] = {0};

void gm7121_init(disp_tv_mode tv_mode);
//extern s32 bsp_disp_set_tv_func(u32 sel, disp_tv_func *func);
s32 tv_i2c_write(u8 sub_addr, u8 data);
s32 tv_i2c_read(u8 sub_addr, u8 *data);
s32 tv_i2c_exit(void);

s32 tv_power_init(void)
{
    s32  ret;

	if(tv_used != 1)
	{
		return 0;
	}

    ret = script_parser_fetch("tv0_para", "tv_power_boot", (int *)tv_power, 16);
	if(ret == 0)
	{
		tv_power_used = 1;
	}
	return 0;

}

s32 gm7121_tv_power_on(u32 on_off)
{
	if(tv_used != 1 || tv_power_used != 1)
	{
		return 0;
	}
	if(on_off == 0)
	{
		OSAL_Power_Disable(tv_power);
	}
	else
	{
		OSAL_Power_Enable(tv_power);
	}
    __wrn("[TV]gm7121_tv_power_on\n");
    return 0;
}

s32 gm7121_tv_open(void)
{
	if(tv_used != 1)
	{
		return 0;
	}
    gm7121_init(g_tv_mode);

    return 0;
}

s32 gm7121_tv_close(void)
{
	if(tv_used != 1)
	{
		return 0;
	}

    tv_i2c_exit();
	return 0;
}

s32 gm7121_tv_get_mode(void)
{
    return (s32)g_tv_mode;
}

s32 gm7121_tv_set_mode(disp_tv_mode tv_mode)
{
    g_tv_mode = tv_mode;

    return 0;
}

s32 gm7121_tv_get_hpd_status(void)
{
    return 0;
}

s32 gm7121_tv_get_mode_support(disp_tv_mode tv_mode)
{
    if(tv_mode == DISP_TV_MOD_PAL || tv_mode == DISP_TV_MOD_NTSC)
        return 1;

    return 0;
}
void gm7121_init(disp_tv_mode tv_mode)
{
    
#if 0
    GM7121_Config(0x00,0x00);    
    GM7121_Config(0x01,0x00);    
    GM7121_Config(0x02,0x00);    
    GM7121_Config(0x03,0x00);    
    GM7121_Config(0x04,0x00);    
    GM7121_Config(0x05,0x00);    
    GM7121_Config(0x06,0x00);    
    GM7121_Config(0x07,0x00);    
    GM7121_Config(0x08,0x00);    
    GM7121_Config(0x09,0x00);    
    GM7121_Config(0x0A,0x00);    
    GM7121_Config(0x0B,0x00);    
    GM7121_Config(0x0C,0x00);    
    GM7121_Config(0x0D,0x00);    
    GM7121_Config(0x0E,0x00);    
    GM7121_Config(0x0F,0x00);    
    GM7121_Config(0x10,0x00);    
    GM7121_Config(0x11,0x00);    
    GM7121_Config(0x12,0x00);    
    GM7121_Config(0x13,0x00);    
    GM7121_Config(0x14,0x00);    
    GM7121_Config(0x15,0x00);    
    GM7121_Config(0x16,0x00);    
    GM7121_Config(0x17,0x00);    
    GM7121_Config(0x18,0x00);    
    GM7121_Config(0x19,0x00);    
    GM7121_Config(0x1A,0x00);    
    GM7121_Config(0x1B,0x00);    
    GM7121_Config(0x1C,0x00);    
    GM7121_Config(0x1D,0x00);    
    GM7121_Config(0x1E,0x00);    
    GM7121_Config(0x1F,0x00);    
    GM7121_Config(0x20,0x00);    
    GM7121_Config(0x21,0x00);    
    GM7121_Config(0x22,0x00);    
    GM7121_Config(0x23,0x00);    
    GM7121_Config(0x24,0x00);    
    GM7121_Config(0x25,0x00);    
    GM7121_Config(0x26,0x1D);    
    GM7121_Config(0x27,0x05); 
#endif
    GM7121_Config(0x28,0x21);    
    GM7121_Config(0x29,0x1D);    
    GM7121_Config(0x2A,0x00);    
    GM7121_Config(0x2B,0x00);    
    GM7121_Config(0x2C,0x00);    
    GM7121_Config(0x2D,0x00);    
    GM7121_Config(0x2E,0x00);    
    GM7121_Config(0x2F,0x00);    
    GM7121_Config(0x30,0x00);    
    GM7121_Config(0x31,0x00);    
    GM7121_Config(0x32,0x00);    
    GM7121_Config(0x33,0x00);    
    GM7121_Config(0x34,0x00);    
    GM7121_Config(0x35,0x00);    
    GM7121_Config(0x36,0x00);    
    GM7121_Config(0x37,0x00);    
    GM7121_Config(0x38,0x00);    
    GM7121_Config(0x39,0x00);    

    //GM7121_Config(0x3A,0x93);    //color  strape 
    GM7121_Config(0x3A,0x13);   //data	主模式
    //*GM7121_Config(0x3A,0x03);     //sync from rcv1 and rcv2   从模式

    GM7121_Config(0x5A,0x00);    
    GM7121_Config(0x5B,0x6d);    
    GM7121_Config(0x5C,0x9f);    
    //GM7121_Config(0x5D,0x1e);  
    GM7121_Config(0x5E,0x1c);    
    GM7121_Config(0x5F,0x35);    
    GM7121_Config(0x60,0x00); 
     
    if (tv_mode == DISP_TV_MOD_PAL)
	{
	    GM7121_Config(0x5D,0x0e);
	    GM7121_Config(0x61,0x06);    //PAL
	    GM7121_Config(0x63,0xCB);    //PAL    
        GM7121_Config(0x64,0x8A);    //PAL 
        GM7121_Config(0x65,0x09);    //PAL  
        GM7121_Config(0x66,0x2A);    //PAL  
	}
	else if (tv_mode == DISP_TV_MOD_NTSC)
	{
	    GM7121_Config(0x5D,0x1e);
        GM7121_Config(0x61,0x01);    //NTSC
        GM7121_Config(0x63,0x1f);    //NTSC
        GM7121_Config(0x64,0x7c);    //NTSC
        GM7121_Config(0x65,0xF0);    //NTSC
        GM7121_Config(0x66,0x21);    //NTSC
	}	   	
	else
	{
	    GM7121_Config(0x5D,0x0e);
	    GM7121_Config(0x61,0x06);    //PAL
	    GM7121_Config(0x63,0xCB);    //PAL    
        GM7121_Config(0x64,0x8A);    //PAL 
        GM7121_Config(0x65,0x09);    //PAL  
        GM7121_Config(0x66,0x2A);    //PAL 
	}
	
	GM7121_Config(0x62,0x3B);     //RTCI Enable	
#if 0        
	GM7121_Config(0x67,0x00);     
    GM7121_Config(0x68,0x00);    
    GM7121_Config(0x69,0x00);    
    GM7121_Config(0x6A,0x00); 
#endif       
    GM7121_Config(0x6B,0x12);    //主模式
    //GM7121_Config(0x6B,0x20);     // 从模式
    //*GM7121_Config(0x6B,0x00);     // 从模式 7121C 20110105
    
    GM7121_Config(0x6C,0x01);    //主模式	
    //GM7121_Config(0x6C,0x96);    //从模式	
    //GM7121_Config(0x6C,0x51);    //从模式	 7121C 20110105
    //*GM7121_Config(0x6C,0x06);    //从模式
    
    GM7121_Config(0x6D,0x20);    //主模式
    //GM7121_Config(0x6D,0x18);    //从模式 
    //GM7121_Config(0x6D,0x11);    //从模式  7121C 20110105
    //*GM7121_Config(0x6D,0x00);    //从模式 
    
    GM7121_Config(0x6E,0x80);    //video with color	
    GM7121_Config(0x6F,0x00);    
    GM7121_Config(0x70,0x14);    
    GM7121_Config(0x71,0x00);    
    GM7121_Config(0x72,0x00);    
    GM7121_Config(0x73,0x00);    
    GM7121_Config(0x74,0x00);    
    GM7121_Config(0x75,0x00);    
    GM7121_Config(0x76,0x00);     
    GM7121_Config(0x77,0x00);    
    GM7121_Config(0x78,0x00);    
    GM7121_Config(0x79,0x00);    
    GM7121_Config(0x7A,0x16);    
    GM7121_Config(0x7B,0x36);    
    GM7121_Config(0x7C,0x40);    
    GM7121_Config(0x7D,0x00);    
    GM7121_Config(0x7E,0x00);    
    GM7121_Config(0x7F,0x00); 
	__wrn("[TV]gm7121_init, tv_Mode=%d\n", tv_mode);
}


#if 0
static int tv_func_init(void)
{
    __disp_tv_func tv_ops;
    tv_ops.tv_power_on = gm7121_tv_power_on;
    tv_ops.tv_open = gm7121_tv_open;
    tv_ops.tv_close = gm7121_tv_close;
    tv_ops.tv_get_hpd_status = gm7121_tv_get_hpd_status;
    tv_ops.tv_set_mode = gm7121_tv_set_mode;
    tv_ops.tv_get_mode_support = gm7121_tv_get_mode_support;
    //bsp_disp_set_tv_func(0, &tv_ops);

    return 0;
}

static int tv_i2c_remove()
{
    return 0;
}
#endif
 


int  tv_i2c_init(void)
{
    s32  val;
    s32  ret;

	if(tv_used != 1)
	{
		return 0;
	}
	
    if(script_parser_fetch("tv0_para", "tv_twi_used", &val,1)< 0){
	   __wrn("##fail to get tv_twi_used config\n");
    }
	else{
        tv_i2c_used = val;
        if(tv_i2c_used == 1)
        {
            ret = script_parser_fetch("tv0_para", "tv_twi_id", &val,1);
            tv_i2c_id = (ret < 0)? tv_i2c_id:val;

            ret = script_parser_fetch("tv0_para", "tv_twi_addr", &val,1);
            normal_i2c[0] = (ret < 0)? normal_i2c[0]:val;
        }
    }
	return 0;
}

int tv_i2c_exit(void)
{
    if(tv_i2c_used){
	   //i2c_exit();
	   return 0;
    }
	return -1;
}

s32 tv_i2c_write(__u8 sub_addr, __u8 data)
{
    if(tv_i2c_used){
	   __s8 ret= -10;
#if defined(CONFIG_SUNXI_I2C)
		ret = i2c_write(tv_i2c_id, normal_i2c[0],sub_addr,1,&data,1);
#else
		printf("err: not complie I2C driver\n");
#endif
		return ret;
    }
	return -1;
}

s32 tv_i2c_read(__u8 sub_addr, __u8 *data)
{
    if(tv_i2c_used){
	   __s8 ret= -10;
#if defined(CONFIG_SUNXI_I2C)
		ret = i2c_read(tv_i2c_id, normal_i2c[0],sub_addr,1,data,1);
#else
		printf("err: not complie I2C driver\n");
#endif
		return ret;
    }
	return -1;
}

int  gm7121_module_init(void)
{
	s32 val;
    if(script_parser_fetch("tv0_para", "tv_used", &val,1)< 0){
       __wrn("##fail to get tv_used config\n");
    }else {
		tv_used = val;
	}
	if(tv_used != 1)
	{
		return 0;
	}
	tv_power_init();
    tv_i2c_init();
	//tv_func_init();
	printf("[TV]gm7121_module_init finish!\n");
	return 0;
}

