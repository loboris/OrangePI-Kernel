/*
 * Copyright (C) 2014 arokux
 *
 * arokux <arokux at gmail.com>
 *
 * Based on code from
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
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

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include "ehci.h"

#define BIT(x)	(1 << (x))

#define SUNXI_USB1_IO_BASE		0x01c14000
#define SUNXI_USB2_IO_BASE		0x01c1c000

#define SUNXI_USB_PMU_IRQ_ENABLE	0x800
#define SUNXI_USB_CSR			0x01c13404
#define SUNXI_USB_PASSBY_EN		1

#define SUNXI_EHCI_AHB_ICHR8_EN		BIT(10)
#define SUNXI_EHCI_AHB_INCR4_BURST_EN	BIT(9)
#define SUNXI_EHCI_AHB_INCRX_ALIGN_EN	BIT(8)
#define SUNXI_EHCI_ULPI_BYPASS_EN	BIT(0)

static struct sunxi_ehci_hcd {
	void *ehci_base;
	struct usb_hcd *hcd;
	int usb_rst_mask;
	int ahb_clk_mask;
	int gpio_vbus;
	void *csr;
	int irq;
	int id;
} sunxi_echi_hcd[CONFIG_USB_MAX_CONTROLLER_COUNT] = {
	[0] = {
		.ehci_base = (void *) SUNXI_USB1_IO_BASE,
		.usb_rst_mask = CCM_USB_CTRL_PHY1_RST,
		.ahb_clk_mask = BIT(AHB_GATE_OFFSET_USB_EHCI0),
		.gpio_vbus = CONFIG_SUNXI_USB_VBUS0_GPIO,
		.csr = (void*) SUNXI_USB_CSR,
		.irq = 39,
		.id = 1,
	},
#if (CONFIG_USB_MAX_CONTROLLER_COUNT > 1)
	[1] = {
		.ehci_base = (void *) SUNXI_USB2_IO_BASE,
		.usb_rst_mask = CCM_USB_CTRL_PHY2_RST,
		.ahb_clk_mask = BIT(AHB_GATE_OFFSET_USB_EHCI1),
		.gpio_vbus = CONFIG_SUNXI_USB_VBUS1_GPIO,
		.csr = (void*) SUNXI_USB_CSR,
		.irq = 40,
		.id = 2,
	}
#endif
};

static int sunxi_gpio_output(u32 pin, u32 val)
{
	u32 dat;
	u32 bank = GPIO_BANK(pin);
	u32 num = GPIO_NUM(pin);
	struct sunxi_gpio *pio =
		&((struct sunxi_gpio_reg *)SUNXI_PIO_BASE)->gpio_bank[bank];

	dat = readl(&pio->dat);
	if (val)
		dat |= 0x1 << num;
	else
		dat &= ~(0x1 << num);

	writel(dat, &pio->dat);

	return 0;
}

static void usb_phy_write(struct sunxi_ehci_hcd *sunxi_ehci, int addr,
			  int data, int len)
{
	int temp = 0, j = 0, usbc_bit = 0;
	void *dest = sunxi_ehci->csr;

	usbc_bit = BIT(sunxi_ehci->id * 2);
	for (j = 0; j < len; j++) {
		/* set the bit address to be written */
		temp = readl(dest);
		temp &= ~(0xff << 8);
		temp |= ((addr + j) << 8);
		writel(temp, dest);

		clrbits_le32(dest, usbc_bit);
		/* set data bit */
		if (data & 0x1)
			temp |= BIT(7);
		else
			temp &= ~BIT(7);
		writeb(temp, dest);

		setbits_le32(dest, usbc_bit);

		clrbits_le32( dest, usbc_bit);

		data >>= 1;
	}
}

static void sunxi_usb_phy_init(struct sunxi_ehci_hcd *sunxi_ehci)
{
	/* The following comments are machine
	 * translated from Chinese, you have been warned!
	 */

	/* adjust PHY's magnitude and rate */
	usb_phy_write(sunxi_ehci, 0x20, 0x14, 5);

	/* threshold adjustment disconnect */
	usb_phy_write(sunxi_ehci, 0x2a, 3, 2);

	return;
}

static void sunxi_usb_passby(struct sunxi_ehci_hcd *sunxi_ehci, int enable)
{
	unsigned long reg_value = 0;
	unsigned long bits = 0;
	void *addr = sunxi_ehci->ehci_base + SUNXI_USB_PMU_IRQ_ENABLE;

	bits = SUNXI_EHCI_AHB_ICHR8_EN |
		SUNXI_EHCI_AHB_INCR4_BURST_EN |
		SUNXI_EHCI_AHB_INCRX_ALIGN_EN |
		SUNXI_EHCI_ULPI_BYPASS_EN;


	reg_value = readl(addr);

	if (enable)
		reg_value |= bits;
	else
		reg_value &= ~bits;

	writel(reg_value, addr);

	return;
}

static void sunxi_ehci_enable(struct sunxi_ehci_hcd *sunxi_ehci)
{
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	setbits_le32(&ccm->usb_clk_cfg, sunxi_ehci->usb_rst_mask);
	setbits_le32(&ccm->ahb_gate0, sunxi_ehci->ahb_clk_mask);

	sunxi_usb_phy_init(sunxi_ehci);

	sunxi_usb_passby(sunxi_ehci, SUNXI_USB_PASSBY_EN);

	/* gpio_direction_output(sunxi_ehci->gpio_vbus, 1); */
	sunxi_gpio_set_cfgpin(sunxi_ehci->gpio_vbus, SUNXI_GPIO_OUTPUT);
	sunxi_gpio_output(sunxi_ehci->gpio_vbus, 1);
}

static void sunxi_ehci_disable(struct sunxi_ehci_hcd *sunxi_ehci)
{
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* gpio_direction_output(sunxi_ehci->gpio_vbus, 0); */
	sunxi_gpio_set_cfgpin(sunxi_ehci->gpio_vbus, SUNXI_GPIO_OUTPUT);
	sunxi_gpio_output(sunxi_ehci->gpio_vbus, 0);

	sunxi_usb_passby(sunxi_ehci, !SUNXI_USB_PASSBY_EN);

	clrbits_le32(&ccm->ahb_gate0, sunxi_ehci->ahb_clk_mask);
	clrbits_le32(&ccm->usb_clk_cfg, sunxi_ehci->usb_rst_mask);
}

int ehci_hcd_init(int index, enum usb_init_type init, struct ehci_hccr **hccr,
		struct ehci_hcor **hcor)
{
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_ehci_hcd *sunxi_ehci = &sunxi_echi_hcd[index];

	/* enable common PHY only once */
	if (index == 0)
		setbits_le32(&ccm->usb_clk_cfg, CCM_USB_CTRL_PHYGATE);

	sunxi_ehci_enable(sunxi_ehci);

	*hccr = sunxi_ehci->ehci_base;

	*hcor = (struct ehci_hcor *)((uint32_t) *hccr
				+ HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	debug("sunxi-ehci: init hccr %x and hcor %x hc_length %d\n",
		(uint32_t)*hccr, (uint32_t)*hcor,
		(uint32_t)HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	return 0;
}

int ehci_hcd_stop(int index)
{
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct sunxi_ehci_hcd *sunxi_ehci = &sunxi_echi_hcd[index];

	sunxi_ehci_disable(sunxi_ehci);

	/* disable common PHY only once, for the last hcd */
	if (index == CONFIG_USB_MAX_CONTROLLER_COUNT - 1)
		clrbits_le32(&ccm->usb_clk_cfg, CCM_USB_CTRL_PHYGATE);

	return 0;
}
