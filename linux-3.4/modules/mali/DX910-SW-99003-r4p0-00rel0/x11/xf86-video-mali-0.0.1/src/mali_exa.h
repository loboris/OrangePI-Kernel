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

#ifndef _MALI_EXA_H_
#define _MALI_EXA_H_

#include "exa.h"
#include <ump/ump.h>
#if !(defined(UMP_VERSION_MAJOR) && UMP_VERSION_MAJOR == 2)
#include <ump/ump_ref_drv.h>
#endif

#include "mali_def.h"

typedef struct
{
	ump_handle handle;
	unsigned long usize;
	unsigned long offset;
} mali_mem_info;

typedef struct
{
	Bool isFrameBuffer;
	int refs;
	int bits_per_pixel;
	unsigned long addr;
	mali_mem_info *mem_info;
	PixmapPtr other_buffer;
} PrivPixmap;

extern Bool maliSetupExa(ScreenPtr pScreen, ExaDriverPtr exa);

#endif /* _MALI_EXA_H_ */
