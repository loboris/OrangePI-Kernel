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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "xorgVersion.h"
#include "xf86.h"
#include "xf86drm.h"
#include "dri2.h"
#include "damage.h"
#include "mali_def.h"
#include "mali_fbdev.h"
#include "mali_exa.h"
#include "mali_dri.h"
#include "damage.h"

typedef struct
{
	PixmapPtr pPixmap;
	Bool isPageFlipped;
	Bool has_bb_reference;
} MaliDRI2BufferPrivateRec, *MaliDRI2BufferPrivatePtr;

static DRI2Buffer2Ptr MaliDRI2CreateBuffer(DrawablePtr pDraw, unsigned int attachment, unsigned int format)
{
	ScreenPtr pScreen = pDraw->pScreen;
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	MaliPtr fPtr = MALIPTR(pScrn);

	DRI2Buffer2Ptr buffer;
	MaliDRI2BufferPrivatePtr privates;
	PixmapPtr pPixmapToWrap = NULL;
	PrivPixmap *privPixmapToWrap;

	PixmapPtr pWindowPixmap = NULL;
	PrivPixmap *privWindowPixmap = NULL;

	if (pDraw->type == DRAWABLE_WINDOW)
	{
		pWindowPixmap = pScreen->GetWindowPixmap((WindowPtr) pDraw);
		privWindowPixmap = (PrivPixmap *)exaGetPixmapDriverPrivate(pWindowPixmap);
	}

	// xf86DrvMsg( pScrn->scrnIndex, X_INFO, "Creating attachment %u around drawable %p (window pixmap %p)\n", attachment, pDraw, pScreen->GetWindowPixmap((WindowPtr)pDraw));
	DEBUG_STR(1, "Creating attachment %u around drawable %p (window pixmap %p)\n", attachment, pDraw, pScreen->GetWindowPixmap((WindowPtr)pDraw));

	buffer = calloc(1, sizeof * buffer);

	if (NULL == buffer)
	{
		return NULL;
	}

	privates = calloc(1, sizeof * privates);

	if (NULL == privates)
	{
		free(buffer);
		return NULL;
	}

	/* initialize privates info to default values */
	privates->pPixmap = NULL;
	privates->isPageFlipped = FALSE;
	privates->has_bb_reference = FALSE;

	/* initialize buffer info to default values */
	buffer->attachment = attachment;
	buffer->driverPrivate = privates;
	buffer->format = format;
	buffer->flags = 0;

	if (DRI2CanFlip(pDraw) && fPtr->use_pageflipping && DRAWABLE_WINDOW == pDraw->type)
	{
		assert(privWindowPixmap->other_buffer != NULL);

		if (DRI2BufferFrontLeft == attachment || DRI2BufferFakeFrontLeft == attachment)
		{
			pPixmapToWrap = pWindowPixmap;
		}
		else if (DRI2BufferBackLeft == attachment)
		{
			PixmapPtr tempPixmap = privWindowPixmap->other_buffer;
			pPixmapToWrap = tempPixmap;
		}

		privates->isPageFlipped = TRUE;
	}

	/* Either the surface isn't swappable or the framebuffer back buffer is already in use */
	if (pPixmapToWrap == NULL)
	{
		if (DRI2BufferFrontLeft == attachment)
		{
			if (DRAWABLE_PIXMAP == pDraw->type)
			{
				pPixmapToWrap = (PixmapPtr)pDraw;
			}
			else
			{
				pPixmapToWrap = pScreen->GetWindowPixmap((WindowPtr) pDraw);
			}

			privPixmapToWrap = (PrivPixmap *)exaGetPixmapDriverPrivate(pPixmapToWrap);
		}
		else
		{
			/* Create a new pixmap for the offscreen data */
			pPixmapToWrap = (*pScreen->CreatePixmap)(pScreen, pDraw->width, pDraw->height, (format != 0) ? format : pDraw->depth, 0);

			if (NULL == pPixmapToWrap)
			{
				xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "[%s:%d] unable to allocate pixmap\n", __FUNCTION__, __LINE__);
				free(buffer);
				free(privates);
				return NULL;
			}

			/* This is the only case where we don't need to add an additional reference to the pixmap, so
			 * drop one here to negate the increase at the end of the function */
			pPixmapToWrap->refcnt--;

			exaMoveInPixmap(pPixmapToWrap);
		}
	}

	privates->pPixmap = pPixmapToWrap;
	privPixmapToWrap = (PrivPixmap *)exaGetPixmapDriverPrivate(pPixmapToWrap);

	buffer->cpp = pPixmapToWrap->drawable.bitsPerPixel / 8;
	buffer->name = ump_secure_id_get(privPixmapToWrap->mem_info->handle);
	buffer->flags = privPixmapToWrap->mem_info->offset;
	buffer->pitch = pPixmapToWrap->devKind;

	if (0 == buffer->pitch)
	{
		//xf86DrvMsg( pScrn->scrnIndex, X_WARNING, "[%s:%d] Warning: Autocalculating pitch\n", __FUNCTION__, __LINE__ );
		WARNING_MSG("Autocalculating pitch");
		buffer->pitch = ((pPixmapToWrap->drawable.width * pPixmapToWrap->drawable.bitsPerPixel) + 7) / 8;
	}

	pPixmapToWrap->refcnt++;

	return buffer;
}

static void MaliDRI2DestroyBuffer(DrawablePtr pDraw, DRI2Buffer2Ptr buffer)
{
	ScreenPtr pScreen = pDraw->pScreen;
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];

	//xf86DrvMsg( pScrn->scrnIndex, X_INFO, "Destroying attachment %d for drawable %p\n", buffer->attachment, pDraw);
	DEBUG_STR(1, "Destroying attachment %d for drawable %p\n", buffer->attachment, pDraw);

	if (NULL != buffer)
	{
		MaliDRI2BufferPrivatePtr private = buffer->driverPrivate;

		if (NULL != private && NULL != private->pPixmap)
		{
			(*pScreen->DestroyPixmap)(private->pPixmap);
		}

		free(private);
		free(buffer);
	}
}

static int wt_set_window_pixmap(WindowPtr win, pointer data)
{
	ScreenPtr pScreen = ((DrawablePtr)win)->pScreen;

	/* We only want to update the window pixmap for non-redirected windows. Otherwise redirected windows will end up
	 * drawing to the front buffer. */
	if (win && win->redirectDraw == RedirectDrawNone)
	{
		pScreen->SetWindowPixmap(win, (PixmapPtr)data);
		return WT_WALKCHILDREN;
	}
	else
	{
		/* Don't walk the children of a redirected window as they won't be marked as redirected but as their
		 * parent is they will be too. If we continue to walk to them they will end up drawing to the front
		 * buffer. */
		return WT_DONTWALKCHILDREN;
	}
}

DrawablePtr dri2_get_drawable(DrawablePtr pDraw, DRI2BufferPtr buffer)
{
	DrawablePtr drawable = NULL;

	if (DRI2BufferFrontLeft == buffer->attachment)
	{
		drawable = pDraw;
	}
	else
	{
		MaliDRI2BufferPrivatePtr private = buffer->driverPrivate;
		drawable = &private->pPixmap->drawable;
	}

	return drawable;
}

PixmapPtr dri2_get_drawable_pixmap(DrawablePtr pDraw)
{
	PixmapPtr pix = NULL;

	if (!pDraw)
	{
		return NULL;
	}

	if (DRAWABLE_WINDOW == pDraw->type)
	{
		pix = pDraw->pScreen->GetWindowPixmap((WindowPtr)pDraw);
	}
	else
	{
		pix = (PixmapPtr)pDraw;
	}

	return pix;
}

static int exchange_buffers(DrawablePtr pDraw, DRI2BufferPtr front, DRI2BufferPtr back, int dri2_complete_cmd)
{
	DrawablePtr front_drawable;
	DrawablePtr back_drawable;
	PixmapPtr front_pixmap;
	PixmapPtr back_pixmap;
	PrivPixmap *front_privPixmap = NULL;
	PrivPixmap *back_privPixmap = NULL;
	Bool exchange_mem_info = FALSE;
	Bool both_framebuffer = FALSE;
	Bool one_framebuffer = FALSE;

	front_drawable = dri2_get_drawable(pDraw, front);
	back_drawable = dri2_get_drawable(pDraw, back);

	front_pixmap = dri2_get_drawable_pixmap(front_drawable);
	back_pixmap = dri2_get_drawable_pixmap(back_drawable);

	front_privPixmap = (PrivPixmap *)exaGetPixmapDriverPrivate(front_pixmap);
	back_privPixmap = (PrivPixmap *)exaGetPixmapDriverPrivate(back_pixmap);

	exchange(front->name, back->name);

	/* exchange the private memory info if
	 * 1. it is a swap between non-framebuffers
	 * exchange the driverPrivate info if
	 * 1. it is a flip between the framebuffers
	 */
	both_framebuffer = (front_privPixmap->isFrameBuffer && back_privPixmap->isFrameBuffer);
	one_framebuffer = (front_privPixmap->isFrameBuffer || back_privPixmap->isFrameBuffer);

	if (both_framebuffer)
	{
		exchange_mem_info = FALSE;
	}
	else if (!one_framebuffer && dri2_complete_cmd == DRI2_EXCHANGE_COMPLETE)
	{
		exchange_mem_info = TRUE;
	}

	if (exchange_mem_info)
	{
		if (front_privPixmap->mem_info->usize == back_privPixmap->mem_info->usize)
		{
			DEBUG_STR(1, "EXCHANGING UMP ID 0x%x with 0x%x (%s)\n", ump_secure_id_get(front_privPixmap->mem_info->handle), ump_secure_id_get(back_privPixmap->mem_info->handle), dri2_complete_cmd == DRI2_EXCHANGE_COMPLETE ? "SWAP" : "FLIP");
			exchange(front_privPixmap->mem_info, back_privPixmap->mem_info);
		}
		else
		{
			DEBUG_STR(1, "EXCHANGING FAILED FOR UMP ID 0x%x size: %ld with 0x%x size (%ld)\n", ump_secure_id_get(front_privPixmap->mem_info->handle), front_privPixmap->mem_info->usize, ump_secure_id_get(back_privPixmap->mem_info->handle), back_privPixmap->mem_info->usize);
			return 0;
		}
	}
	else if (both_framebuffer)
	{
		exchange(front->driverPrivate, back->driverPrivate);
	}

	return 1;
}

static void platform_wait_for_vsync(ScrnInfoPtr pScrn, int fb_lcd_fd)
{
#if PLATFORM_ORION
	int interrupt = 1;

	if (ioctl(fb_lcd_fd, S3CFB_SET_VSYNC_INT, &interrupt) < 0)
	{
		//xf86DrvMsg( pScrn->scrnIndex, X_WARNING, "[%s:%d] failed in S3CFB_SET_VSYNC_INT\n", __FUNCTION__, __LINE__ );
		WARNING_MSG("failed in S3CFB_SET_VSYNC_INT");
	}

	if (ioctl(fb_lcd_fd, FBIO_WAITFORVSYNC, 0) < 0)
	{
		//xf86DrvMsg( pScrn->scrnIndex, X_WARNING, "[%s:%d] failed in FBIO_WAITFORVSYNC\n", __FUNCTION__, __LINE__ );
		WARNING_MSG("failed in FBIO_WAITFORVSYNC");
	}

	interrupt = 0;

	if (ioctl(fb_lcd_fd, S3CFB_SET_VSYNC_INT, &interrupt) < 0)
	{
		//xf86DrvMsg( pScrn->scrnIndex, X_WARNING, "[%s:%d] failed in S3CFB_SET_VSYNC_INT\n", __FUNCTION__, __LINE__ );
		WARNING_MSG("failed in S3CFB_SET_VSYNC_INT");
	}

#endif
}

static void MaliDRI2CopyRegion(DrawablePtr pDraw, RegionPtr pRegion, DRI2BufferPtr pDstBuffer, DRI2BufferPtr pSrcBuffer)
{
	GCPtr pGC;
	RegionPtr copyRegion;
	ScreenPtr pScreen = pDraw->pScreen;
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	MaliPtr fPtr = MALIPTR(pScrn);
	PixmapPtr winPixmap;

	MaliDRI2BufferPrivatePtr srcPrivate = pSrcBuffer->driverPrivate;
	MaliDRI2BufferPrivatePtr dstPrivate = pDstBuffer->driverPrivate;

	PixmapPtr srcPixmap = srcPrivate->pPixmap;
	PixmapPtr dstPixmap = dstPrivate->pPixmap;

	DrawablePtr srcDrawable = &srcPrivate->pPixmap->drawable;
	DrawablePtr dstDrawable = pDraw;

	DEBUG_STR(1, "Blitting!");

	pGC = GetScratchGC(pDraw->depth, pScreen);
	copyRegion = REGION_CREATE(pScreen, NULL, 0);
	REGION_COPY(pScreen, copyRegion, pRegion);
	(*pGC->funcs->ChangeClip)(pGC, CT_REGION, copyRegion, 0);
	ValidateGC(dstDrawable, pGC);
	(*pGC->ops->CopyArea)(srcDrawable, dstDrawable, pGC, 0, 0, pDraw->width, pDraw->height, 0, 0);
	FreeScratchGC(pGC);
}

/*
 * MaliDRI2ScheduleSwap is the implementation of DRI2SwapBuffers, this function
 * should wait for vblank event which will trigger registered event handler.
 * Event handler will do FLIP/SWAP/BLIT according to event type.
 *
 * Current DRM doesn't support vblank well, so this function just do FLIP/
 *       SWAP/BLIT directly, according to drawable information.
 */
static int MaliDRI2ScheduleSwap(ClientPtr client, DrawablePtr pDraw, DRI2BufferPtr front,
                                DRI2BufferPtr back, CARD64 *target_msc, CARD64 divisor,
                                CARD64 remainder, DRI2SwapEventPtr func, void *data)
{
	ScreenPtr pScreen = pDraw->pScreen;
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	MaliPtr fPtr = MALIPTR(pScrn);
	BoxRec box;
	RegionRec region;
	void *tmp;
	int dri2_complete_cmd = DRI2_BLIT_COMPLETE;

	MaliDRI2BufferPrivatePtr front_priv = front->driverPrivate;
	MaliDRI2BufferPrivatePtr back_priv  = back->driverPrivate;

	PixmapPtr front_pixmap = front_priv->pPixmap;
	PixmapPtr back_pixmap  = back_priv->pPixmap;
	assert(front_pixmap != NULL);
	assert(back_pixmap  != NULL);

	PrivPixmap *front_pixmap_priv = (PrivPixmap *)exaGetPixmapDriverPrivate(front_pixmap);
	PrivPixmap *back_pixmap_priv  = (PrivPixmap *)exaGetPixmapDriverPrivate(back_pixmap);

	if (DRI2CanFlip(pDraw) && fPtr->use_pageflipping && DRAWABLE_WINDOW == pDraw->type && front_priv->isPageFlipped)
	{

		unsigned int line_length = fPtr->fb_lcd_var.xres * fPtr->fb_lcd_var.bits_per_pixel / 8;
		fPtr->fb_lcd_var.yoffset = back_pixmap_priv->mem_info->offset / line_length;
		DEBUG_STR(1, "Flipping! ofs %d\n", fPtr->fb_lcd_var.yoffset);

		if (ioctl(fPtr->fb_lcd_fd, FBIOPAN_DISPLAY, &fPtr->fb_lcd_var) == -1)
		{
			//xf86DrvMsg( pScrn->scrnIndex, X_WARNING, "[%s:%d] failed in FBIOPAN_DISPLAY\n", __FUNCTION__, __LINE__ );
			WARNING_MSG("failed in FBIOPAN_DISPLAY");
		}

		if (fPtr->use_pageflipping_vsync)
		{
			platform_wait_for_vsync(pScrn, fPtr->fb_lcd_fd);
		}

		ioctl(fPtr->fb_lcd_fd, FBIOGET_VSCREENINFO, &fPtr->fb_lcd_var);

		dri2_complete_cmd = DRI2_FLIP_COMPLETE;
		exchange_buffers(pDraw, front, back, dri2_complete_cmd);

		/* Tell the X server that all 2D rendering should be done to newPixmap from now on */
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1, 9, 4, 901, 0)
		pScreen->SourceValidate(pDraw, 0, 0, pDraw->width, pDraw->height);
#else
		pScreen->SourceValidate(pDraw, 0, 0, pDraw->width, pDraw->height, IncludeInferiors);
#endif

		pScreen->SetScreenPixmap(back_pixmap);

		/* Update all windows so that their front buffer is now the other half of the fbdev */
		WalkTree(pScreen, wt_set_window_pixmap, back_pixmap);


	}
	else if (front_pixmap->drawable.width        == back_pixmap->drawable.width   &&
	         front_pixmap->drawable.height       == back_pixmap->drawable.height  &&
	         front_pixmap->drawable.bitsPerPixel == back_pixmap->drawable.bitsPerPixel)
	{
		PixmapPtr dst_pix = dri2_get_drawable_pixmap(dri2_get_drawable(pDraw, front));

		dri2_complete_cmd = DRI2_EXCHANGE_COMPLETE;

		if (exchange_buffers(pDraw, front, back, dri2_complete_cmd))
		{
			DEBUG_STR(1, "Swapping! front_pixmap->drawable.width %d, front_pixmap->drawable.height %d, pDraw->width %d, pDraw->height %d\n", front_pixmap->drawable.width, front_pixmap->drawable.height, pDraw->width, pDraw->height);
			box.x1 = 0;
			box.y1 = 0;
			box.x2 = pDraw->width;
			box.y2 = pDraw->height;
			REGION_INIT(pScreen, &region, &box, 0);

			RegionTranslate(&region, dst_pix->screen_x, dst_pix->screen_y);

			DamageDamageRegion(pDraw, &region);

			front_pixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER ;
			back_pixmap->drawable.serialNumber = NEXT_SERIAL_NUMBER ;
		}
		else
		{
			box.x1 = 0;
			box.y1 = 0;
			box.x2 = pDraw->width;
			box.y2 = pDraw->height;
			REGION_INIT(pScreen, &region, &box, 0);

			MaliDRI2CopyRegion(pDraw, &region, front, back);
			dri2_complete_cmd = DRI2_BLIT_COMPLETE;
		}
	}
	else
	{
		box.x1 = 0;
		box.y1 = 0;
		box.x2 = pDraw->width;
		box.y2 = pDraw->height;
		REGION_INIT(pScreen, &region, &box, 0);

		MaliDRI2CopyRegion(pDraw, &region, front, back);
		dri2_complete_cmd = DRI2_BLIT_COMPLETE;
	}

	DRI2SwapComplete(client, pDraw, 0, 0, 0, dri2_complete_cmd, func, data);

	/* Adjust returned value */
	*target_msc += 1;


	return TRUE;
}

Bool MaliDRI2ScreenInit(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	MaliPtr fPtr = MALIPTR(pScrn);
	DRI2InfoRec info;
	int dri2_major = 2, dri2_minor = 0, i;
	struct stat sbuf;
	dev_t d;
	char *p;
#if DRI2INFOREC_VERSION >= 4
	const char *driverNames[1];
#endif

	if (xf86LoaderCheckSymbol("DRI2Version"))
	{
		DRI2Version(&dri2_major, &dri2_minor);
	}

	if (dri2_minor < 1)
	{
		//xf86DrvMsg( pScrn->scrnIndex, X_ERROR, "%s requires DRI2 module version 1.1.0 or later\n", __func__);
		ERROR_MSG("%s requires DRI2 module version 1.1.0 or later\n", __func__);
		return FALSE;
	}

	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "DRI2 version: %i.%i\n", dri2_major, dri2_minor);
	INFO_MSG("DRI2 version: %i.%i\n", dri2_major, dri2_minor);


	/* extract deviceName */
	info.fd = fPtr->drm_fd;
	fstat(info.fd, &sbuf);
	d = sbuf.st_rdev;

	p = fPtr->deviceName;

	for (i = 0; i < DRM_MAX_MINOR; i++)
	{
		sprintf(p, DRM_DEV_NAME, DRM_DIR_NAME, i);

		if (stat(p, &sbuf) == 0 && sbuf.st_rdev == d)
		{
			break;
		}
	}

	if (i == DRM_MAX_MINOR)
	{
		//xf86DrvMsg( pScrn->scrnIndex, X_ERROR, "%s failed to open drm device\n", __func__ );
		ERROR_MSG("%s failed to open drm device\n", __func__);
		return FALSE;
	}


	info.driverName = "Mali DRI2";
	info.deviceName = p;

#if DRI2INFOREC_VERSION == 1
	info.version = 1;
	info.CreateBuffers = MaliDRI2CreateBuffers;
	info.DestroyBuffers = MaliDRI2DestroyBuffers;
#elif DRI2INFOREC_VERSION == 2
	info.version = 2;
	info.CreateBuffer = MaliDRI2CreateBuffer;
	info.DestroyBuffer = MaliDRI2DestroyBuffer;
#else
	info.version = 3;
	info.CreateBuffer = MaliDRI2CreateBuffer;
	info.DestroyBuffer = MaliDRI2DestroyBuffer;
#endif

	info.CopyRegion = MaliDRI2CopyRegion;

#if DRI2INFOREC_VERSION >= 4

	if (fPtr->use_pageflipping)
	{
		info.version = 4;
		info.ScheduleSwap = MaliDRI2ScheduleSwap;
		info.GetMSC = NULL;
		info.ScheduleWaitMSC = NULL;
		info.numDrivers = 1;
		info.driverNames = driverNames;
		driverNames[0] = info.driverName;
	}

#endif

	if (FALSE == DRI2ScreenInit(pScreen, &info))
	{
		return FALSE;
	}

	return TRUE;
}

void MaliDRI2CloseScreen(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	MaliPtr fPtr = MALIPTR(pScrn);

	DRI2CloseScreen(pScreen);

	fPtr->dri_render = DRI_NONE;
}
