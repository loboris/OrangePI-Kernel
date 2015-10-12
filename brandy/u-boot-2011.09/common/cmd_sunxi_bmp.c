/*
 * (C) Copyright 2002
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * BMP handling routines
 */

#include <common.h>
#include <bmp_layout.h>
#include <command.h>
#include <malloc.h>
#include <bat.h>
#include <sunxi_advert.h>
#include <sys_config.h>

static int sunxi_bmp_probe_info (uint addr);
static int sunxi_bmp_show(sunxi_bmp_store_t bmp_info);

struct __advert_head	advert_head;

DECLARE_GLOBAL_DATA_PTR;

static __s32 check_sum(void *mem_base, __u32 size, __u32 src_sum)
{
	__u32 *buf = (__u32 *)mem_base;
	__u32 count = 0;
	__u32 sum = 0;
	__u32 last 	= 0;
	__u32 curlen = 0;
	__s32 i = 0;

	/* 生成校验和 */
	count = size >> 2;                         // 以 字（4bytes）为单位计数

	//16字节对齐
	do
	{
		sum += *buf++;                         // 依次累加，求得校验和
		sum += *buf++;                         // 依次累加，求得校验和
		sum += *buf++;                         // 依次累加，求得校验和
		sum += *buf++;                         // 依次累加，求得校验和
	}while( ( count -= 4 ) > (4-1) );

	//4字节对齐
	for (i = 0; i < count; i++)
	{
		sum += *buf++;
	}

	//如果有1 2 3字节的尾巴，则处理尾巴数据，按照lsb的格式
	curlen = size % 4;
	if((size & 0x03) != 0)
	{
		memcpy(&last, mem_base + size - curlen, curlen);
		sum += last;	//加上尾巴补全的u32数据
	}

	printf("sum=%x\n", sum);
	printf("src_sum=%x\n", src_sum);

	if( sum == src_sum )
		return 0;               // 校验成功
	else
		return -1;             // 校验失败
}

/*
 * Allocate and decompress a BMP image using gunzip().
 *
 * Returns a pointer to the decompressed image data. Must be freed by
 * the caller after use.
 *
 * Returns NULL if decompression failed, or if the decompressed data
 * didn't contain a valid BMP signature.
 */

static int do_sunxi_bmp_info(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	uint addr;

	if(argc == 2)
	{
		/* use argument only*/
		addr = simple_strtoul(argv[1], NULL, 16);
		debug("bmp addr=%x\n", addr);
	}
	else if(argc == 3)
	{
		char  load_addr[8];
		char  filename[32];
		char *const bmp_argv[6] = { "fatload", "sunxi_flash", "0", load_addr, filename, NULL };

		addr = simple_strtoul(argv[1], NULL, 16);
		memcpy(load_addr, argv[1], 8);
		memset(filename, 0, 32);
		memcpy(filename, argv[2], strlen(argv[2]));
#ifdef DEBUG
	    int i;

		for(i=0;i<6;i++)
		{
	        printf("argv[%d] = %s\n", i, argv[i]);
		}
#endif
	    if(do_fat_fsload(0, 0, 5, bmp_argv))
		{
		   printf("sunxi bmp info error : unable to open bmp file %s\n", argv[2]);

		   return cmd_usage(cmdtp);
	    }
	}
	else
	{
		return cmd_usage(cmdtp);
	}

	return (sunxi_bmp_probe_info(addr));
}

U_BOOT_CMD(
	sunxi_bmp_info,	3,	1,	do_sunxi_bmp_info,
	"manipulate BMP image data",
	"only one para : the address where the bmp stored\n"
);


static int do_sunxi_bmp_display(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	uint addr;
	uint de_addr;
	sunxi_bmp_store_t bmp_info;

	if(argc == 2)
	{
		/* use argument only*/
		addr = simple_strtoul(argv[1], NULL, 16);
#if defined(CONFIG_SUNXI_LOGBUFFER)
		de_addr = CONFIG_SYS_SDRAM_BASE + gd->ram_size - SUNXI_DISPLAY_FRAME_BUFFER_SIZE;
#else
		de_addr = SUNXI_DISPLAY_FRAME_BUFFER_ADDR;
#endif
	}
	else if(argc == 3)
	{
		addr = simple_strtoul(argv[1], NULL, 16);
		de_addr = simple_strtoul(argv[2], NULL, 16);
	}
	else if(argc == 4)
	{
		char  load_addr[8];
		char  filename[32];
		char *const bmp_argv[6] = { "fatload", "sunxi_flash", "0", load_addr, filename, NULL };

		addr = simple_strtoul(argv[1], NULL, 16);
		memcpy(load_addr, argv[1], 8);
		memset(filename, 0, 32);
		memcpy(filename, argv[3], strlen(argv[3]));
		de_addr = simple_strtoul(argv[2], NULL, 16);
#ifdef DEBUG
	    int i;

		for(i=0;i<6;i++)
		{
	        printf("argv[%d] = %s\n", i, argv[i]);
		}
#endif
	    if(do_fat_fsload(0, 0, 5, bmp_argv))
		{
		   printf("sunxi bmp info error : unable to open bmp file %s\n", argv[2]);

		   return cmd_usage(cmdtp);
	    }
	}
	else
	{
		return cmd_usage(cmdtp);
	}
	if(de_addr < CONFIG_SYS_SDRAM_BASE)
	{
#if defined(CONFIG_SUNXI_LOGBUFFER)
		de_addr = CONFIG_SYS_SDRAM_BASE + gd->ram_size - SUNXI_DISPLAY_FRAME_BUFFER_SIZE;
#else
		de_addr = SUNXI_DISPLAY_FRAME_BUFFER_ADDR;
#endif
	}
	debug("bmp addr %x, display addr %x\n", addr, de_addr);
	bmp_info.buffer = (void *)de_addr;
	if(!sunxi_bmp_decode(addr, &bmp_info))
	{
		debug("decode bmp ok\n");

		return sunxi_bmp_show(bmp_info);
	}
	debug("decode bmp error\n");

	return -1;
}

U_BOOT_CMD(
	sunxi_bmp_show,	4,	1,	do_sunxi_bmp_display,
	"manipulate BMP image data",
	"sunxi_bmp_display addr [de addr]\n"
	"parameters 1 : the address where the bmp stored\n"
	"parameters 2 : option para, the address where the bmp display\n"
);

/*
 * Subroutine:  sunxi_bmp_display
 *
 * Description: Show bmp file in device
 *
 * Inputs:	name of file that in bootloader
 *
 * Return:      None
 *
 */
int sunxi_bmp_display(char *name)
{
#ifdef CONFIG_FPGA
    return 0;
#else
	sunxi_bmp_store_t bmp_info;
	char  bmp_name[32];
	memset(bmp_name, 0, 32);
	strcpy(bmp_name, name);

#ifndef USE_AW_FAT
	char *const bmp_argv[6] = { "fatload", "sunxi_flash", "0", "40000000", bmp_name, NULL };
    if(do_fat_fsload(0, 0, 5, bmp_argv))
	{
	   printf("sunxi bmp info error : unable to open logo file %s\n", bmp_argv[4]);

	   return -1;
    }
#else
	char *const bmp_argv[4] = {"bootloader", bmp_name, "40000000", NULL};
  	if(do_aw_fat_fsload(0, 0, 4, bmp_argv))
	{
	   printf("sunxi bmp info error : unable to open logo file %s\n", bmp_argv[1]);

	   return -1;
    }
#endif

	//bmp_info.buffer = (void *)SUNXI_DISPLAY_FRAME_BUFFER_ADDR;
#if defined(CONFIG_SUNXI_LOGBUFFER)
	bmp_info.buffer = (void *)(CONFIG_SYS_SDRAM_BASE + gd->ram_size - SUNXI_DISPLAY_FRAME_BUFFER_SIZE);
#else
	bmp_info.buffer = (void *)(SUNXI_DISPLAY_FRAME_BUFFER_ADDR);
#endif
	if(!sunxi_bmp_decode(0x40000000, &bmp_info))
	{
		debug("decode bmp ok\n");

		return sunxi_bmp_show(bmp_info);
	}

	return -1;
#endif
}
/*
 * Subroutine:  bmp_info
 *
 * Description: Show information about bmp file in memory
 *
 * Inputs:	addr		address of the bmp file
 *
 * Return:      None
 *
 */
static int sunxi_bmp_probe_info(uint addr)
{
	bmp_image_t *bmp=(bmp_image_t *)addr;

	if((bmp->header.signature[0]!='B') || (bmp->header.signature[1]!='M'))
	{
		printf("this is not a bmp picture\n");

		return -1;
	}
	debug("bmp picture dectede\n");

	printf("Image size    : %d x %d\n", bmp->header.width, (bmp->header.height & 0x80000000) ? (-bmp->header.height):(bmp->header.height));
	printf("Bits per pixel: %d\n", bmp->header.bit_count);

	return(0);
}

/*
 * Subroutine:  bmp_display
 *
 * Description: Display bmp file located in memory
 *
 * Inputs:	addr		address of the bmp file
 *
 * Return:      None
 *
 */
int sunxi_bmp_decode(unsigned long addr, sunxi_bmp_store_t *bmp_info)
{
	char *tmp_buffer;
	char *bmp_data;
	int zero_num = 0;
	bmp_image_t *bmp = (bmp_image_t *)addr;
	int x, y, bmp_bpix;

	if((bmp->header.signature[0]!='B') || (bmp->header.signature[1] !='M'))
	{
		printf("this is not a bmp picture\n");

		return -1;
	}
	debug("bmp dectece\n");

	bmp_bpix = bmp->header.bit_count/8;
	if((bmp_bpix != 3) && (bmp_bpix != 4))
	{
		printf("no support bmp picture without bpix 24 or 32\n");

		return -1;
	}
	if(bmp_bpix ==3)
	{		
		zero_num = (4 - ((3*bmp->header.width) % 4))&3;
	}
	debug("bmp bitcount %d\n", bmp->header.bit_count);
	x = bmp->header.width;
	y = (bmp->header.height & 0x80000000) ? (-bmp->header.height):(bmp->header.height);
	debug("bmp x = %x, bmp y = %x\n", x, y);

	tmp_buffer = (char *)bmp_info->buffer;
	bmp_data = (char *)(addr + bmp->header.data_offset);
	if(bmp->header.height & 0x80000000)
    {
	      if(zero_num == 0)
                {
                    memcpy(tmp_buffer,bmp_data,x*y*bmp_bpix);
                }
                else
                {
                    int i, line_bytes, real_line_byte;	
	            char *src;
	            line_bytes = (x * bmp_bpix) + zero_num;
		    real_line_byte = x * bmp_bpix;
		    for(i=0; i<y; i++)
                   {
             	    src = bmp_data + i*line_bytes;
                     memcpy(tmp_buffer, src, real_line_byte);
                    tmp_buffer += real_line_byte;
                    }
                }
    }
    else
    {
    	uint i, line_bytes, real_line_byte;
        char *src;

		line_bytes = (x * bmp_bpix) + zero_num;
		real_line_byte = x * bmp_bpix;
		for(i=0; i<y; i++)
        {
        	src = bmp_data + (y - i - 1) * line_bytes;
        	memcpy(tmp_buffer, src, real_line_byte);
            tmp_buffer += real_line_byte;
        }
    }
    bmp_info->x = x;
    bmp_info->y = y;
    bmp_info->bit = bmp->header.bit_count;
	flush_cache((uint)bmp_info->buffer, x * y * bmp_bpix);

	return 0;
}

static int sunxi_bmp_show(sunxi_bmp_store_t bmp_info)
{
	debug("begin to set framebuffer\n");
	if(board_display_framebuffer_set(bmp_info.x, bmp_info.y, bmp_info.bit, (void *)bmp_info.buffer))
	{
		printf("sunxi bmp display error : set frame buffer error\n");

		return -2;
	}
	debug("begin to show layer\n");
	board_display_show(0);
	debug("bmp display finish\n");

	return 0;
}

int do_sunxi_logo(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	return sunxi_bmp_display("bootlogo.bmp");
}

U_BOOT_CMD(
	logo,	1,	0,	do_sunxi_logo,
	"show default logo",
	"no args\n"
);

static int sunxi_advert_verify_magic(unsigned long addr)
{
	u32 length = 0;

	memcpy((u32 *)&advert_head, (u32 *)addr, sizeof(struct __advert_head));
	if(strcmp((char *)advert_head.magic, ADVERT_MAGIC))
	{
		printf("advert magic not equal,%s\n", (char *)advert_head.magic);
		return -1;
	}

	length = advert_head.length;
	if((length = 0) || (length > 64*1024*1024))
	{
		printf("advert length=%d to big or to short\n", length);
		return -1;
	}

	return 0;
}

static int sunxi_advert_verify_head(char *fatname, char *filename)
{
	char bmp_name[32];
	char fat_name[32];

	memset(bmp_name, 0, 32);
	strcpy(bmp_name, filename);

	memset(fat_name, 0, 32);
	strcpy(fat_name, fatname);

	char *const bmp_argv[4] = {fat_name, bmp_name, "40000000", NULL};
	if(do_aw_fat_fsload(0, 0, 4, bmp_argv))
	{
		printf("sunxi bmp info error : unable to open file %s\n", bmp_argv[1]);
		return -1;
	}

	return sunxi_advert_verify_magic(0x40000000);
}

int sunxi_advert_display(char *fatname, char *filename)
{
#ifndef USE_AW_FAT
	printf("please define USE_AW_FAT before used\n");
	return -1;
#endif

	if(sunxi_advert_verify_head(fatname, "advert.crc"))
	{
		printf("check advert magic failed\n");
		return -1;
	}

	sunxi_bmp_store_t bmp_info;
	char bmp_name[32];
	char fat_name[32];

	memset(bmp_name, 0, 32);
	strcpy(bmp_name, filename);

	memset(fat_name, 0, 32);
	strcpy(fat_name, fatname);
	char *const bmp_argv[4] = {fat_name, bmp_name, "40000000", NULL};
	if(do_aw_fat_fsload(0, 0, 4, bmp_argv))
	{
		printf("sunxi bmp info error : unable to open logo file %s\n", bmp_argv[1]);
		return -1;
	}

#if defined(CONFIG_SUNXI_LOGBUFFER)
	bmp_info.buffer = (void *)(CONFIG_SYS_SDRAM_BASE + gd->ram_size - SUNXI_DISPLAY_FRAME_BUFFER_SIZE);
#else
	bmp_info.buffer = (void *)(SUNXI_DISPLAY_FRAME_BUFFER_ADDR);
#endif

	if(!check_sum((u32 *)0x40000000, advert_head.length, advert_head.check_sum))
	{
		debug("check_sum advert bmp ok\n");
		if(!sunxi_bmp_decode(0x40000000, &bmp_info))
		{
			debug("decode bmp ok\n");
			return sunxi_bmp_show(bmp_info);
		}
	}
	return -1;
}

int sunxi_advert_disp_probe(void)
{
	int ret;
	int advert_disp = 0;
	ret = script_parser_fetch("boot_disp", "advert_disp", &advert_disp, 1);
	if(!ret && advert_disp)
	{
		printf("adver need show\n");
		return 0;
	}
	printf("adver not need show\n");
	return -1;
}

int do_sunxi_advert(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	return sunxi_advert_display("Reserve0", "advert.bmp");
}

U_BOOT_CMD(
	advert,	1,	0,	do_sunxi_advert,
	"show default advert",
	"no args\n"
);