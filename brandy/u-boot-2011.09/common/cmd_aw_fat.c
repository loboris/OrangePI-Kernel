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
#include <sys_partition.h>
#include "../fs/aw_fs/ff.h"

#define FILE_MAX_SIZE (10 * 1024 * 1024)

int aw_fat_fsload(char *part_name, char *file_name, char *load_addr, int len)
{
	FATFS ud_mount;
	FILE  ud_fs;
	int ret = 0;
	unsigned int read_bytes = 0;
	
	f_mount(0, &ud_mount, part_name);
	ret = f_open (&ud_fs, file_name, FA_READ);
	if(ret != FR_OK)
	{
		printf("error: open %s, maybe it is not exist\n", file_name);
		return 0;
	}
	
	if (!f_read(&ud_fs, load_addr, len, &read_bytes))
	{
		printf("read byte = %d\n", read_bytes);
		return read_bytes;
	}
	f_close(&ud_fs);
	f_mount(0, NULL, NULL);
	return 0;
}

int do_aw_fat_fsload(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
/*	FATFS ud_mount;
	FIL  ud_fs;
	int ret = 0;
*/
	char *src_addr = NULL;
	//unsigned int read_bytes;
	
	if (argc < 4) {
		printf( "usage: <part> <filename> <addr> [bytes]\n");
		return 1;
	}
	src_addr = (char *)simple_strtoul(argv[2], NULL, 16);
	if (!aw_fat_fsload(argv[0], argv[1], src_addr, FILE_MAX_SIZE))
	{
		return 1;
	}
	return 0;

}

U_BOOT_CMD(
	aw_fatload,	4,	0,	do_aw_fat_fsload,
	"load binary file from a dos filesystem",
	"<part> <filename> <addr> <bytes>\n"
	"    - load binary file 'filename' from partition\n"
	"      to address 'addr' from dos filesystem"
);
