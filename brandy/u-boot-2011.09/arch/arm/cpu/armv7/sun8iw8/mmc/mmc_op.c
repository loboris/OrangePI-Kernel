/*
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * Description: MMC driver for general mmc operations
 * Author: Aaron <leafy.myeh@allwinnertech.com>
 * Date: 2012-2-3 14:18:18
 */
#include "common.h"
#include "private_uboot.h"
#include "mmc_def.h"
#include "mmc.h"

static unsigned bootcard_offset;

int sunxi_mmc_init(int sdc_no, unsigned bus_width, normal_gpio_cfg *gpio_info, int offset);
extern int sunxi_mmc_exit(int sdc_no, normal_gpio_cfg *gpio_info, int offset);

__s32 SDMMC_PhyInit(__u32 card_no, __u32 bus_width, normal_gpio_cfg *gpio_info, int offset);
{
	int ret = 0;
	ret = sunxi_mmc_init(card_no, bus_width, gpio_info, offset);
	if ( ret <= 0) {
		mmcdbg("Init SD/MMC card failed !\n");
		return -1;
	}

	return ret;
}

__s32 SDMMC_PhyExit(__u32 card_no, normal_gpio_cfg *gpio_info, int offset);
{
	sunxi_mmc_exit(card_no, gpio_info, offset);
    return 0;
}

__s32 SDMMC_PhyRead(__u32 start_sector, __u32 nsector, void *buf, __u32 card_no)
{
	return mmc_bread(card_no, start_sector, nsector, buf);
}

__s32 SDMMC_PhyWrite(__u32 start_sector, __u32 nsector, void *buf, __u32 card_no)
{
	//return mmc_bwrite(card_no, start_sector, nsector, buf);
	mmcinfo("Don't implement %s for 24k!\n",__FUNCTION__);
	return -1;
}

__s32 SDMMC_PhyDiskSize(__u32 card_no)
{
	struct mmc *mmc = find_mmc_device(card_no);
	return mmc->lba;
}

__s32 SDMMC_PhyErase(__u32 block, __u32 nblock, __u32 card_no)
{
	//return mmc_berase(card_no, block, nblock);
	mmcinfo("Don't implement %s for 24k!\n",__FUNCTION__);
	return -1;
}

__s32 SDMMC_LogicalInit(__u32 card_no, __u32 card_offset, __u32 bus_width)
{
	bootcard_offset = card_offset;
    return SDMMC_PhyInit(card_no, bus_width);
}

__s32 SDMMC_LogicalExit(__u32 card_no)
{
	bootcard_offset = 0;
    return SDMMC_PhyExit(card_no);
}

__s32 SDMMC_LogicalRead(__u32 start_sector, __u32 nsector, void *buf, __u32 card_no)
{
	return mmc_bread(card_no, start_sector + bootcard_offset, nsector, buf);
}

__s32 SDMMC_LogicalWrite(__u32 start_sector, __u32 nsector, void *buf, __u32 card_no)
{
	//return mmc_bwrite(card_no, start_sector + bootcard_offset, nsector, buf);
	mmcinfo("Don't implement %s for 24k!\n",__FUNCTION__);
	return -1;
}

__s32 SDMMC_LogicalDiskSize(__u32 card_no)
{
	return SDMMC_PhyDiskSize(card_no) - bootcard_offset;
}

__s32 SDMMC_LogicaErase(__u32 block, __u32 nblock, __u32 card_no)
{
	//return mmc_berase(card_no, block + bootcard_offset, nblock);
	mmcinfo("Don't implement %s for 24k!\n",__FUNCTION__);
	return -1;
}

void OSAL_CacheRangeFlush(void*Address, __u32 Length, __u32 Flags)
{
	;
}

void set_mmc_para(int smc_no,void *sdly_addr )
{
	struct spare_boot_head_t  *uboot_buf = (struct spare_boot_head_t *)CONFIG_SYS_TEXT_BASE;

	memcpy((void *)uboot_buf->boot_data.sdcard_spare_data, sdly_addr, sizeof(struct tuning_sdly));
	printf("%s,sdly 50M %d\n",__func__, ((struct tuning_sdly *)(uboot_buf->boot_data.sdcard_spare_data))->sdly_50M);
	printf("%s,sdly 25M %d\n",__func__, ((struct tuning_sdly *)(uboot_buf->boot_data.sdcard_spare_data))->sdly_25M);
	return;
}
