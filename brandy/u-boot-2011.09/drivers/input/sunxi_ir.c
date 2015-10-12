/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Charles <yanjianbo@allwinnertech.com>
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
#include <asm/arch/platform.h>
#include <sys_config.h>
#include "sunxi_ir.h"
#include <asm/io.h>
#include <asm/arch/intc.h>

#define TOUCH_KEY_COUNT		3

unsigned long intsta;
unsigned int key_code, code_valid;
unsigned int check_key_code, key_code_count = 0;

static struct ir_raw_buffer	ir_rawbuf;

static inline void ir_reset_rawbuffer(void)
{
	ir_rawbuf.dcnt = 0;
}

static inline void ir_write_rawbuffer(unsigned char data)
{
	if (ir_rawbuf.dcnt < IR_RAW_BUF_SIZE){ 
		ir_rawbuf.buf[ir_rawbuf.dcnt++] = data;
		//printf("data%d = 0x%x\n", ir_rawbuf.dcnt, data);
	}
	else
		printf("ir_write_rawbuffer: IR Rx Buffer Full!!\n");
}

static inline unsigned char ir_read_rawbuffer(void)
{
	unsigned char data = 0x00;

	if(ir_rawbuf.dcnt > 0)
		data = ir_rawbuf.buf[--ir_rawbuf.dcnt];

	return data;
}

static inline int ir_rawbuffer_empty(void)
{
	return (ir_rawbuf.dcnt == 0);
}

static inline int ir_rawbuffer_full(void)
{
	return (ir_rawbuf.dcnt >= IR_RAW_BUF_SIZE);
}

static inline unsigned char ir_get_data(void)
{
	return (unsigned char)(readl(IR_BASE + IR_RXDAT_REG));
}

static inline unsigned long ir_get_intsta(void)
{
	return (readl(IR_BASE + IR_RXINTS_REG));
}

static inline void ir_clr_intsta(unsigned long bitmap)
{
	unsigned long tmp = readl(IR_BASE + IR_RXINTS_REG);

	tmp &= ~0xff;
	tmp |= bitmap&0xff;
	writel(tmp, IR_BASE + IR_RXINTS_REG);
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static int ir_code_valid(unsigned long code)
{
	unsigned long tmp1, tmp2;
	unsigned int ir_addr_code;
	int ret = -1;

#ifdef IR_CHECK_ADDR_CODE
	/* Check Address Value */
	
	ret = script_parser_fetch("ir_boot_para", "ir_addr_code", (int *)&ir_addr_code, sizeof(int) / 4);
	if (!ret) {
		if ((code&0xffff) != (ir_addr_code&0xffff))
			return 0; /* Address Error */
	
		tmp1 = code & 0x00ff0000;
		tmp2 = (code & 0xff000000)>>8;
		return ((tmp1^tmp2)==0x00ff0000);  /* Check User Code */
	}
	else {
		return 0;		
	}
	
#else	
	/* Do Not Check Address Value */
	tmp1 = code & 0x00ff00ff;
	tmp2 = (code & 0xff00ff00)>>8;
	
	//return ((tmp1^tmp2)==0x00ff00ff);
	return (((tmp1^tmp2) & 0x00ff0000)==0x00ff0000 );
#endif /* #ifdef IR_CHECK_ADDR_CODE */
}


static unsigned long ir_packet_handler(unsigned char *buf, unsigned long dcnt)
{
	unsigned long len;
	unsigned char val = 0x00;
	unsigned char last = 0x00;
	unsigned long code = 0;
	int bitCnt = 0;
	unsigned long i=0;

	//printf("buf_byte_count = %d \n", (int)dcnt);
	
	/* Find Lead '1' */
	len = 0;
	for (i=0; i<dcnt; i++) {
		val = buf[i];
		if (val & 0x80) {
			len += val & 0x7f;
		} else {
			if (len > IR_L1_MIN)
				break;
			
			len = 0;
		}
	}

	if ((val&0x80) || (len<=IR_L1_MIN))
		return IR_ERROR_CODE; /* Invalid Code */
		
	/* Find Lead '0' */
	len = 0;
	for (; i<dcnt; i++) {
		val = buf[i];		
		if (val & 0x80) {
			if(len > IR_L0_MIN)
				break;
			
			len = 0;
		} else {
			len += val & 0x7f;
		}		
	}
	
	if ((!(val&0x80)) || (len<=IR_L0_MIN))
		return IR_ERROR_CODE; /* Invalid Code */
	
	/* go decoding */
	code = 0;  /* 0 for Repeat Code */
	bitCnt = 0;
	last = 1;
	len = 0;
	for (; i<dcnt; i++) {
		val = buf[i];		
		if (last) {
			if (val & 0x80) {
				len += val & 0x7f;
			} else {
				if (len > IR_PMAX) {		/* Error Pulse */
					return IR_ERROR_CODE;
				}
				last = 0;
				len = val & 0x7f;
			}
		} else {
			if (val & 0x80) {
				if (len > IR_DMAX){		/* Error Distant */
					return IR_ERROR_CODE;
				} else {
					if (len > IR_DMID)  {
						/* data '1'*/
						code |= 1<<bitCnt;
					}
					bitCnt ++;
					if (bitCnt == 32)
						break;  /* decode over */
				}	
				last = 1;
				len = val & 0x7f;
			} else {
				len += val & 0x7f;
			}
		}
	}
	
	return code;
}


/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void ir_irq_service(void *data)
{
	unsigned long intsta = ir_get_intsta();
	unsigned long rxint = readl(IR_BASE + IR_RXINTE_REG);
	unsigned long tmp;
	
	//printf("IR IRQ Serve\n");

	ir_clr_intsta(intsta);
	
	//if(intsta & (IR_RXINTS_RXDA|IR_RXINTS_RXPE))  /*FIFO Data Valid*/
	/*Read Data Every Time Enter this Routine*/ 
	{
		unsigned long dcnt =  (ir_get_intsta()>>8) & 0x1f;
		unsigned long i = 0;
		
		/* Read FIFO */
		for (i=0; i<dcnt; i++) {
			if (ir_rawbuffer_full()) {
				printf("ir_irq_service: Raw Buffer Full!!\n");
				break;
			} 
			else {
				ir_write_rawbuffer(ir_get_data());
			}
		}
	}

	if (intsta & IR_RXINTS_RXPE) {	 /* Packet End */
		{
			unsigned long dcnt =  (ir_get_intsta()>>8) & 0x1f;
			unsigned long i = 0;
			
			/* Read FIFO */
			for (i=0; i<dcnt; i++) {
				if (ir_rawbuffer_full()) {
					printf("ir_irq_service: Raw Buffer Full!!\n");
					break;
				} 
				else {
					ir_write_rawbuffer(ir_get_data());
				}
			}
		}
		
		key_code = ir_packet_handler(ir_rawbuf.buf, ir_rawbuf.dcnt);
		ir_rawbuf.dcnt = 0;
		code_valid = ir_code_valid(key_code);
		//printf("[IR] code 0x%08x, code_valid %d\n", (u32)key_code, code_valid);
		
		//add by charles
		//uboot阶段，盒子上电启动会检测红外，按3次同一个按键，当一次有效值，这样避免误操作
		if (code_valid) {
			key_code_count++;
			if (key_code != check_key_code) {
				check_key_code = key_code;
				key_code_count = 1;	
			}
			if (key_code_count >= TOUCH_KEY_COUNT) {
				/*disable IR Mode*/
				tmp &= ~(0x3<<4);
				writel(tmp, IR_BASE+IR_CTRL_REG);
				irq_disable(AW_IRQ_CIR0);
				irq_free_handler(AW_IRQ_CIR0);
				tick_printf("[IR] key_code 0x%x, touch %d time\n", key_code, key_code_count);
			}
		}
	}
	
	if (intsta & IR_RXINTS_RXOF) {  /* FIFO Overflow */
		/* flush raw buffer */
		ir_reset_rawbuffer();	
		printf("ir_irq_service: Rx FIFO Overflow!!\n");
	}	
	
	return ;
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static void ir_clk_cfg(void)
{
	unsigned int tmp;
#if (defined CONFIG_ARCH_SUN7I)
	
	//Enable APB Clock for IR
	tmp = readl(CCM_BASE + 0x68);
	tmp |= 0x1 << 6;  				//IR
	writel(tmp, CCM_BASE + 0x68);

	//config Special Clock for IR	(24/8=3MHz)
	tmp = readl(CCM_BASE + 0xB0);
	tmp &= ~(0x3 << 24);		//Select 24MHz
	tmp |=  (0x1 << 31);   	//Open Clock
	tmp &= ~(0x3 << 16);
	tmp |=  (0x3 << 16);		//Divisor = 8
	writel(tmp, CCM_BASE + 0xB0);
/*
#elif (defined CONFIG_ARCH_SUN6I)



*/

#endif /* CONFIG_AW_FPGA_PLATFORM */
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static void ir_sys_cfg(void)
{
	int ret = -1;
	user_gpio_set_t     ir_gpio;
	
	ret = script_parser_fetch("ir_boot_para", "ir_rx", (int *)&ir_gpio, sizeof(user_gpio_set_t)/4);
	if (!ret) {
		gpio_request(&ir_gpio, 1);
	}
	else{
		printf("[ir_boot_para]: no ir_gpio config\n");
	}
	ir_clk_cfg();
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static void ir_reg_cfg(void)
{
	unsigned long tmp = 0;

	/*Enable IR Mode*/
	tmp = 0x3<<4;
	writel(tmp, IR_BASE+IR_CTRL_REG);

	tmp = 0x1<<0; 	//Fsample = 3MHz/128 = 23437.5Hz (42.7us)

	tmp |= (IR_RXFILT_VAL&0x3f)<<2;	//Set Filter Threshold
	tmp |= (IR_RXIDLE_VAL&0xff)<<8; //Set Idle Threshold
	writel(tmp, IR_BASE+IR_SPLCFG_REG);

	/*Invert Input Signal*/
	writel(0x1<<2, IR_BASE+IR_RXCFG_REG);

	/*Clear All Rx Interrupt Status*/
	writel(0xff, IR_BASE+IR_RXINTS_REG);

	/*Set Rx Interrupt Enable*/
	tmp = (0x1<<4)|0x3;
	//tmp |= ((IR_FIFO_SIZE>>1)-1)<<8; //Rx FIFO Threshold = FIFOsz/2;
	tmp |= ((IR_FIFO_SIZE)-1)<<8; //Rx FIFO Threshold = FIFOsz/2;
	writel(tmp, IR_BASE+IR_RXINTE_REG);

	/*Enable IR Module*/
	tmp = readl(IR_BASE+IR_CTRL_REG);
	tmp |= 0x3;
	writel(tmp, IR_BASE+IR_CTRL_REG);

	return;
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int ir_setup(void)
{
	int ir_used = 0;
	int ret = -1;
	
	ret = script_parser_fetch("ir_boot_para", "ir_used", &ir_used, sizeof(int) / 4);
	if (!ret && ir_used) {
		printf("[IR]: init start\n");	
		ir_reset_rawbuffer();	
		ir_sys_cfg();	
		ir_reg_cfg();
		
		irq_enable(AW_IRQ_CIR0);
		irq_install_handler(AW_IRQ_CIR0, ir_irq_service, NULL);

		__msdelay(1000);  //延时为了多点时间检测按键
	}
	else {
		printf("[ir_boot_para]:the ir is no use\n");
	}
	return 0;
}

/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
#define ONEKEY_USB_RECOVERY_MODE            (0x01)
#define ONEKEY_SPRITE_RECOVERY_MODE   	    (0x02)
#define USB_RECOVERY_KEY_VALUE              (0x81)
#define SPRITE_RECOVERY_KEY_VALUE           (0X82)

int ir_fetch_key(void)
{
	unsigned int tmp;
	int ret;
	int recovery_key = 0;
	int true_key = 0;
	int mode;

	/*disable IR Mode*/
	tmp &= ~(0x3<<4);
	writel(tmp, IR_BASE+IR_CTRL_REG);
	irq_disable(AW_IRQ_CIR0);
	irq_free_handler(AW_IRQ_CIR0);
			
	if (key_code_count >= TOUCH_KEY_COUNT)
	{
		ret = script_parser_fetch("ir_boot_para", "ir_recovery_key", &recovery_key, sizeof(int) / 4);
		if (ret)
		{
			printf("[ir_boot_para] no config recovery_key\n");
			return -1;
		}

		//printf("true_ke 0x%x\n", (key_code >> 16) & 0xff);
		true_key = (key_code >> 16) & 0xff;
		if(true_key == recovery_key)
		{
			ret = script_parser_fetch("ir_boot_para", "ir_mode", &mode, sizeof(int) / 4);
			if (ret)
			{
				mode = ONEKEY_USB_RECOVERY_MODE;
			}
			if (mode == ONEKEY_USB_RECOVERY_MODE)
			{
				return mode;
			}
			else if (mode == ONEKEY_SPRITE_RECOVERY_MODE)
			{
				return mode;
			}
			else
			{
				printf("[IR] touch the key, but no mode\n");
				return 0;
			}
		}	
	}
	return -1;	
}
