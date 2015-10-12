/*
**********************************************************************************************************************
*
*						           the Embedded Secure Bootloader System
*
*
*						       Copyright(C), 2006-2014, Allwinnertech Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      :
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#include "common.h"
#include <asm/arch/smta.h>
#include <asm/arch/smc.h>
#include <private_toc.h>
#include <private_uboot.h>

extern sbrom_toc0_config_t *toc0_config;

void secure_switch_unsecure(u32 run_addr, u32 para_addr);
void secure_switch_other(u32 run_addr, u32 para_addr);

unsigned int go_exec (u32 run_addr, u32 para_addr, int out_secure)
{
	if(out_secure)
	{
		//切换到非安全模式
		asm volatile("stmfd sp!, {r0, r1}");

		struct spare_boot_head_t *bfh = (struct spare_boot_head_t *)run_addr;
		toc0_private_head_t *toc0 = (toc0_private_head_t *)CONFIG_SBROMSW_BASE;
		int boot_type = toc0->platform[0];
		uint dram_size;


		if(!boot_type)
		{
			boot_type = 1;
		}
		else if(boot_type == 1)
		{
			boot_type = 0;
		}else if(boot_type == 2){
			//char  storage_data[384];  // 0-159,存储nand信息；160-255,存放卡信息^M
			set_mmc_para(2,(void *)(toc0_config->storage_data+160));
		}

		printf("storage_type=%d\n", boot_type);
		bfh->boot_data.storage_type = boot_type;
                if(out_secure == SECURE_SWITCH_NORMAL)
                {
                    bfh->boot_data.secureos_exist = 1;
		    sunxi_smta_set_to_ns(0);
                }
		dram_size = toc0_config->dram_para[4] & 0xffff;
                if(out_secure == SECURE_SWITCH_NORMAL)
                {
	    	    printf("dram =%d M, reserved size = %d M\n", dram_size, toc0_config->secure_dram_mbytes);
		    sunxi_smc_config(dram_size, toc0_config->secure_dram_mbytes);
		    printf("switch to ns\n");
		    memcpy(bfh->boot_data.dram_para, toc0_config->dram_para, 32 * 4);
		    bfh->boot_data.dram_para[4] -= toc0_config->secure_dram_mbytes;
                }
                else
                {
                    printf("still in secure world  \n");
                    printf("dram = %d M \n",dram_size);
                    bfh->boot_data.secureos_exist = 0;
		    memcpy(bfh->boot_data.dram_para, toc0_config->dram_para, 32 * 4);
                }
#ifdef CONFIG_BOOT_A15
		bfh->boot_data.reserved[0] = toc0_config->boot_cpu;
#endif
		asm volatile("ldmfd sp!, {r0, r1}");
                if(out_secure == SECURE_SWITCH_NORMAL)
		    asm volatile("bx %0"::"r" (secure_switch_unsecure));
                else
		    asm volatile("blx %0"::"r" (secure_switch_other));

	}
	else
	{
		asm volatile("blx %0"::"r" (secure_switch_other));
	}

	return 0;
}

