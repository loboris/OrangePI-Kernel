 #
 # Copyright (C) 2010 ARM Limited. All rights reserved.
 
 # Permission is hereby granted, free of charge, to any person obtaining a copy
 # of this software and associated documentation files (the "Software"), to deal
 # in the Software without restriction, including without limitation the rights
 # to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 # copies of the Software, and to permit persons to whom the Software is
 # furnished to do so, subject to the following conditions
 
 # The above copyright notice and this permission notice shall be included in
 # all copies or substantial portions of the Software.
 
 # THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 # IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 # FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 # AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 # LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 # OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 # THE SOFTWARE.
 # /

MALI_DDK="/work/trunk"

mali_drv_la_LTLIBRARIES = mali_drv.la
mali_drv_la_LDFLAGS = -module -avoid-version -L$(MALI_DDK)/lib -lMali -lUMP -lpthread
mali_drv_ladir = @moduledir@/drivers

AM_CFLAGS = @XORG_CFLAGS@ \
	-I/usr/include/libdrm \
	-I$(MALI_DDK)/include \
	-I$(MALI_DDK)/internal/include/khronos \
	-I$(MALI_DDK)/src/ump/include \
	-I$(MALI_DDK)/src/devicedrv

mali_drv_la_SOURCES = \
	mali_dri.c \
	mali_exa.c \
	mali_fbdev.c \
	mali_lcd.c
