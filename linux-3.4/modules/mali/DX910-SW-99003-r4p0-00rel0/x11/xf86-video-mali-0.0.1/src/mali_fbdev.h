/*
 * Copyright (C) 2010 ARM Limited. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _MALI_FBDEV_DRIVER_H_
#define _MALI_FBDEV_DRIVER_H_

#include <linux/fb.h>
#include "mali_def.h"

#include "xf86.h"
#include "exa.h"

typedef struct
{
	unsigned char  *fbstart;
	unsigned char  *fbmem;
	int             fboff;
	int             lineLength;
	CreateScreenResourcesProcPtr CreateScreenResources;
	void (*PointerMoved)(int index, int x, int y);
	CloseScreenProcPtr  CloseScreen;
	EntityInfoPtr       pEnt;
	OptionInfoPtr       Options;
	int    fb_lcd_fd;
	struct fb_fix_screeninfo fb_lcd_fix;
	struct fb_var_screeninfo fb_lcd_var;
	ExaDriverPtr exa;
	int  dri_render;
	Bool dri_open;
	int  drm_fd;
	char deviceName[64];
	Bool use_pageflipping;
	Bool use_pageflipping_vsync;
	Bool use_cached_ump;
	Bool use_umplock;
	int fd_umplock;
} MaliRec, *MaliPtr;

typedef struct
{
	char *device;
	int   fd;
	void *fbmem;
	unsigned int   fbmem_len;
	unsigned int   fboff;
	struct fb_fix_screeninfo fix;
	struct fb_var_screeninfo var;
	struct fb_var_screeninfo saved_var;
	DisplayModeRec buildin;
	int xres;
	int yres;
} MaliHWRec, *MaliHWPtr;

#define MALIPTR(p) ((MaliPtr)((p)->driverPrivate))
#define MALIHWPTRLVAL(p) (p)->privates[malihwPrivateIndex].ptr
#define MALIHWPTR(p) ((MaliHWPtr)(MALIHWPTRLVAL(p)))

#endif /* _MALI_FBDEV_DRIVER_H_ */

