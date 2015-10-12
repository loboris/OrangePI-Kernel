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
void boot0_jump(unsigned int addr)
{
	//asm volatile("mrs r1, cpsr");
	//asm volatile("bic r1, r1, #(1<<5)");
	//asm volatile("bic r1, r1, #(1<<24)");
	//asm volatile("msr spsr_cxsf, r1");
	asm volatile("mov r2, #0");
    asm volatile("mcr p15, 0, r2, c7, c5, 6");

	asm volatile("bx r0");
}


