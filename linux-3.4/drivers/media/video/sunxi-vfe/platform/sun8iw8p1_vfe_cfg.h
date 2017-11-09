
/* 
 ***************************************************************************************
 * 
 * sun8iw8p1_vfe_cfg.h
 * 
 * Hawkview ISP - sun8iw8p1_vfe_cfg.h module
 * 
 * Copyright (c) 2014 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 * 
 * Version		  Author         Date		    Description
 * 
 *   2.0		  Yang Feng   	2014/07/24	      Second Version
 * 
 ****************************************************************************************
 */

#ifndef _SUN8IW8P1_VFE_CFG_H_
#define _SUN8IW8P1_VFE_CFG_H_


#define VFE_ISP_REGULATOR				""
#define VFE_CSI_REGULATOR				""
#define USE_SPECIFIC_CCI
#define CSI0_CCI_REG_BASE				0x01cb3000

#define CSI0_REGS_BASE          				0x01cb0000
#define MIPI_CSI0_REGS_BASE    			0x01cb1000	//fix
#define CSI1_REGS_BASE          				0x01cb4000

#define ISP_REGS_BASE           				0x01cb8000
#define GPIO_REGS_VBASE					0xf1c20800
#define CPU_DRAM_PADDR_ORG 			0x40000000
#define HW_DMA_OFFSET					0x00000000
#define MAX_VFE_INPUT   					2     //the maximum number of input source of video front end
#define VFE_CORE_CLK						CSI1_S_CLK
#define VFE_CORE_CLK_SRC				PLL_ISP_CLK
#define VFE_MASTER_CLK0					CSI0_M_CLK
#define VFE_MASTER_CLK1					CSI1_M_CLK
#define VFE_MASTER_CLK_24M_SRC		HOSC_CLK
#define VFE_MASTER_CLK_PLL_SRC		PLL_PERIPH0_CLK
#define VFE_MIPI_DPHY_CLK				MIPICSI_CLK
#define VFE_MIPI_DPHY_CLK_SRC			PLL_PERIPH0_CLK
#define VFE_MIPI_CSI_CLK					VFE_CLK_NOT_EXIST 
#define VFE_MIPI_CSI_CLK_SRC			VFE_CLK_NOT_EXIST
#define VFE_VPU_CLK						VFE_CLK_NOT_EXIST
#define VFE_MISC_CLK						CSI0_S_CLK

//set vfe core clk base on sensor size
#define CORE_CLK_RATE_FOR_2M (108*1000*1000)
#define CORE_CLK_RATE_FOR_3M (216*1000*1000)
#define CORE_CLK_RATE_FOR_5M (216*1000*1000)
#define CORE_CLK_RATE_FOR_8M (432*1000*1000)
#define CORE_CLK_RATE_FOR_16M (432*1000*1000)



//CSI & ISP size configs

#define CSI0_REG_SIZE               0x1000
#define MIPI_CSI_REG_SIZE           0x1000
#define MIPI_DPHY_REG_SIZE          0x1000
#define CSI0_CCI_REG_SIZE          0x1000
#define CSI1_REG_SIZE               0x1000
#define CSI1_CCI_REG_SIZE          0x1000
#define ISP_REG_SIZE                0x1000
#define ISP_LOAD_REG_SIZE           0x1000
#define ISP_SAVED_REG_SIZE          0x1000

//ISP size configs

//stat size configs

#define ISP_STAT_TOTAL_SIZE         0x2100

#define ISP_STAT_HIST_MEM_SIZE      0x0200
#define ISP_STAT_AE_MEM_SIZE        0x0600
#define ISP_STAT_AWB_MEM_SIZE       0x0200
#define ISP_STAT_AF_MEM_SIZE        0x0500
#define ISP_STAT_AFS_MEM_SIZE       0x0200
#define ISP_STAT_AWB_WIN_MEM_SIZE   0x1000

#define ISP_STAT_HIST_MEM_OFS       0x0
#define ISP_STAT_AE_MEM_OFS         (ISP_STAT_HIST_MEM_OFS + ISP_STAT_HIST_MEM_SIZE)
#define ISP_STAT_AWB_MEM_OFS        (ISP_STAT_AE_MEM_OFS   + ISP_STAT_AE_MEM_SIZE)
#define ISP_STAT_AF_MEM_OFS         (ISP_STAT_AWB_MEM_OFS  + ISP_STAT_AWB_MEM_SIZE)
#define ISP_STAT_AFS_MEM_OFS        (ISP_STAT_AF_MEM_OFS   + ISP_STAT_AF_MEM_SIZE)
#define ISP_STAT_AWB_WIN_MEM_OFS    (ISP_STAT_AFS_MEM_OFS   + ISP_STAT_AFS_MEM_SIZE)

//table size configs

#define ISP_LINEAR_LUT_LENS_GAMMA_MEM_SIZE 0xE00
#define ISP_LUT_MEM_SIZE            0x0
#define ISP_LENS_MEM_SIZE           0x0600
#define ISP_GAMMA_MEM_SIZE          0x0200
#define ISP_LINEAR_MEM_SIZE          0x0600


#define ISP_DRC_DISC_MEM_SIZE            0x0600
#define ISP_DRC_MEM_SIZE            0x0200
#define ISP_DISC_MEM_SIZE          0x0400

#define MAX_CH_NUM      4
#define MAX_INPUT_NUM   2     //the maximum number of device connected to the same bus
#define MAX_ISP_STAT_BUF  5   //the maximum number of isp statistic buffer
#define MAX_SENSOR_DETECT_NUM  3   //the maximum number of detect sensor on the same bus.

#define MAX_AF_WIN_NUM 1
#define MAX_AE_WIN_NUM 1



#endif /*_SUN8IW8P1_VFE_CFG_H_*/



