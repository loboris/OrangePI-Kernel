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



1. Introduction

---------------
The purpose of this document is to give a very brief overview of the X.Org
components closely connected to the Mali DDK, and how the Mali DDK can be 
integrated into an existing an X.Org environment.


2. Architecture overview
------------------------
The X Windowing system is also commonly known as XFree86, X, X11 and X.Org.

X.Org is the current name of the project. The X Windowing system is based
on a client-server model, and provides management of native resources, such
as windows, pixmaps, and handling of input device control. X is primarily a
protocol and graphics primitive definition, and contains as such no
specifications on how to create a user interface design. Such details are
handled by "Window Managers", "GUI toolkits", "Desktop Environments" and other
application specific graphical user interfaces.

X is a modular stack, and the Mali DDK integration will be done at the X Display
Driver level, as well as the kernel device driver level.

 Mali DDK
 ----------------
|EGL,Base,GLES,VG|
 ----------------
        |
 DDX    |
 ------------------               -----
| EXA, DRI2, FBDEV | <---------> | UMP |
 ------------------               -----
        ^
        |
        |
 -----------------                -----
|   X.Org Server  | <----------> | DRM |
 -----------------                -----

Figure 1: Architecture overview

The above figure tries to depict an overview of the components interesting with
respect to Mali DDK integration.

2.1 UMP
UMP (Unified Memory Provider) provides a way to share both existing and new
memory areas across processes and hardware units.

UMP is a kernel device driver accompanied with a userspace library.

2.2 DRM
DRM (Direct Rendering Manager) is a component of the Direct Rendering
Infrastructure, a generic DRM driver, and another which has specific support for
the video hardware. This pair of drivers allow a userspace client direct access
to the video hardware.

As the device specific driver, "Mali DRM" is provided.

2.3 EXA
EXA was designed to replace XAA in the X.Org server, with initial aims to "extend
the life of venerable XFree86 video drivers". EXA provides hooks for accelerating
parts of the 2D operationis in the X Display Driver.

These hooks include:
- pixmap allocation and management
- solid fills
- blitting operations
  * within screen memory
  * to and from system memory
- porter-Duff compositing and transform operations


It is assumed that the vendor either have a 2D HW unit for which to integrate
into EXA - or alternatively, use software optimized fallbacks for these
operations. The "pixman" library already provides reasonably optimized
operations.

2.4 DRI2
DRI2 (Direct Rendering Infrastructure 2) is a vital part for using native
resources as renderable surfaces with the Mali GPU. DRI2 allows Mali DDK to set
up rendering directly into offscreen and onscreen windows and pixmaps.


3. X.Org Display driver
----------------------
"xf86-video-mali" is provided as a basis for creating your own X Display
Driver. It requires a recent version of the xorg-server, as well as a
successfull integration of UMP with your display device driver.

A reference configuration file, xorg.conf is provided. Please place this 
file in your configuration folder, typically /etc/X11/.

In order to utilize page flipping for fullscreen OpenGL ES and OpenVG 
applications, make sure that you configure your kernel display driver to 
support at least twice the amount of virtual y-resolution as the physical 
resolution.

For a device with a 800x480 display, this means that you will have to
reserve at least 800x960 for your framebuffer memory.


4. Integrating and building
---------------------------

4.1 Integrating UMP with your display driver
It is required that you support the "GET_UMP_SECURE_ID" ioctl for your display
driver. This ioctl has to return a UMP secure ID for your framebuffer memory.

Such an integration typically consists of two major parts
- Mapping and UnMapping of physical blocks of memory
- Returning UMP Secure ID from within ioctl handler

A typical example of mapping the framebuffer memory would look like this:
ump_dd_handle ump_wrapped_buffer;
ump_dd_physical_block ump_memory_description;

ump_memory_description.addr = fix->smem_start;
ump_memory_description.size = buffer_size;
ump_wrapped_buffer = ump_dd_handle_create_from_phys_blocks(
                      &ump_memory_description, 1);

A typical example of retrieving the secure ID from this memory look like this:
u32 __user *psecureid = (u32 __user *) arg;
ump_secure_id secure_id;

secure_id = ump_dd_secure_id_get( ump_wrapped_buffer );
return put_user( (unsigned int)secure_id, psecureid );

4.2 Integrating EXA with your 2D hardware
The EXA module provides hooks for accelerating various 2D operations. Please
refer to the EXA documentation for further information regarding this.

The provided "xf86-video-mali" driver contains an EXA module which has been
integrated with the UMP system. Your 2D driver may therefore require an
integration with UMP as well. The suggestion is to pass the secure ID down to
the kernel device driver for your hardware, but it is also possible to get the
CPU-mapped address for the memory by calling ump_mapped_pointer_get.

Please refer to UMP documentation for more information regarding this.

4.3 Building the Mali DDK for X.Org
The client API libraries can be built to support X.Org by including
"linux-x11-ump" in the variant string. An example of such a variant string can
be VARIANT="mali400-gles20-gles11-linux-x11-ump"

You will also have to edit driver/Makefile_x11.mak to reflect your system. A
complete development setup for X Window System is required, including
development header files and precompiled libraries.

The process of building the Mali DDK, including client API libraries and device
drivers are otherwise the same as building for regular Linux.

Copy your libraries into the system library folder, typically /usr/lib or 
/usr/local/lib. Make sure that file permissions allows read and execute.

4.4 Building the X Display Driver
As for the Mali DDK, the X Display Driver "xf86-video-mali" requires an existing 
development setup for the X Windowing System for your architecture.

Modify the GET_UMP_SECURE_ID ioctl define in mali_exa.h to reflect your 
integration with the display driver.
Also modify the build.sh script inside the X Display Driver package to reflect your
current include paths and library folders for X Window System.

The build.sh shell script performs the following steps:
1. make distclean
2. runs configure script to generate makefile(s)
3. runs make

The display driver can be found in xf86-video-mali-0.0.1/src/.lib/mali_drv.so.
Copy this driver into your X Server display driver path, typically
/usr/lib/xorg/modules/drivers

Make sure that the file has the correct permissions.

A reference xorg.conf file can be found under xf86-video-mali-0.0.1/xorg.conf.
Make sure that you are using "mali" as the Driver under the "Device" section.

Supported options under "Device" section for Mali are:
> fbdev           Select which framebuffer device to use.    Defalt: /dev/fb0
> DRI2            Enable DRI2 or not.                        Default: false
> DRI2_PAGE_FLIP  Enable flipping for fullscreen gles apps.  Default: false
> DRI2_WAIT_VSYNC Enable vsync for fullscreen gles apps.     Default: false


4.5 Building the Mali DRM
The Mali DRM can be plugged into the drivers/gpu/drm folder of your kernel. It
is compatible with the latest 2.6.36 kernel where PCI and AGP bus requirements
were removed in favor of supporting a generic platform device.
Please refer to the to Mali DRM README.txt for more information on 
including the Mali DRM as a platform device.
