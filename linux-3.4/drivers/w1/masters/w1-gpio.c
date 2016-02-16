/*
 * w1-gpio - GPIO w1 bus master driver
 *
 * Copyright (C) 2007 Ville Syrjala <syrjala@sci.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/w1-gpio.h>
#include <linux/gpio.h>

#include "../w1.h"
#include "../w1_int.h"

static int w1_gpio_pullup = 0;
module_param_named(pullup, w1_gpio_pullup, int, 0);

static void w1_gpio_write_bit_dir(void *data, u8 bit)
{
	struct w1_gpio_platform_data *pdata = data;

        //printk(KERN_INFO "W1_GPIO: write_bit_dir %d,%d\n", pdata->pin, bit);
	if (bit)
		gpio_direction_input(pdata->pin);
	else
		gpio_direction_output(pdata->pin, 0);
}

static void w1_gpio_write_bit_val(void *data, u8 bit)
{
	struct w1_gpio_platform_data *pdata = data;

        //printk(KERN_INFO "W1_GPIO: write_bit_data %d,%d\n", pdata->pin, bit);
	__gpio_set_value(pdata->pin, bit);
}

static u8 w1_gpio_read_bit(void *data)
{
        u8 pin_val;
	struct w1_gpio_platform_data *pdata = data;

	pin_val = __gpio_get_value(pdata->pin) ? 1 : 0;
        //printk(KERN_INFO "W1_GPIO: read_bit %d,%d\n", pdata->pin, pin_val);
        return pin_val;
}

static void w1_gpio_bitbang_pullup(void *data, u8 on)
{
	struct w1_gpio_platform_data *pdata = data;

	if (on)
		gpio_direction_output(pdata->pin, 1);
	else
		gpio_direction_input(pdata->pin);
}

static int __init w1_gpio_probe(struct platform_device *pdev)
{
	struct w1_bus_master *master;
	struct w1_gpio_platform_data *pdata; // = pdev->dev.platform_data;
	int err;
        
        pdata = dev_get_platdata(&pdev->dev);

	if (!pdata) {
                printk(KERN_ERR "W1-GPIO: no configuration pdata");
		return -ENXIO;
        }
        
	master = kzalloc(sizeof(struct w1_bus_master), GFP_KERNEL);
	if (!master) {
                printk(KERN_ERR "W1-GPIO: Out of memory");
		return -ENOMEM;
        }
        
	err = gpio_request(pdata->pin, "w1");
	if (err) {
                printk(KERN_ERR "W1_GPIO: gpio request for pin %d failed\n", pdata->pin);
		goto free_master;
        }
        
	master->data = pdata;
	master->read_bit = w1_gpio_read_bit;

	if (pdata->is_open_drain) {
		gpio_direction_output(pdata->pin, 1);
		master->write_bit = w1_gpio_write_bit_val;
	} else {
		gpio_direction_input(pdata->pin);
		master->write_bit = w1_gpio_write_bit_dir;
	}

	if (w1_gpio_pullup)
		if (pdata->is_open_drain)
			printk(KERN_ERR "w1-gpio 'pullup' option "
				"doesn't work with open drain GPIO\n");
		else
			master->bitbang_pullup = w1_gpio_bitbang_pullup;

	err = w1_add_master_device(master);
	if (err)
		goto free_gpio;

	if (pdata->enable_external_pullup)
		pdata->enable_external_pullup(1);

	platform_set_drvdata(pdev, master);

        printk(KERN_INFO "W1_GPIO: added w1 master on GPIO%d\n", pdata->pin);
	return 0;

 free_gpio:
	gpio_free(pdata->pin);
 free_master:
	kfree(master);

	return err;
}

static int __exit w1_gpio_remove(struct platform_device *pdev)
{
	struct w1_bus_master *master = platform_get_drvdata(pdev);
	struct w1_gpio_platform_data *pdata = pdev->dev.platform_data;

	if (pdata->enable_external_pullup)
		pdata->enable_external_pullup(0);

	w1_remove_master_device(master);
	gpio_free(pdata->pin);
	kfree(master);

	return 0;
}

#ifdef CONFIG_PM

static int w1_gpio_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct w1_gpio_platform_data *pdata = pdev->dev.platform_data;

	if (pdata->enable_external_pullup)
		pdata->enable_external_pullup(0);

	return 0;
}

static int w1_gpio_resume(struct platform_device *pdev)
{
	struct w1_gpio_platform_data *pdata = pdev->dev.platform_data;

	if (pdata->enable_external_pullup)
		pdata->enable_external_pullup(1);

	return 0;
}

#else
#define w1_gpio_suspend	NULL
#define w1_gpio_resume	NULL
#endif

static struct platform_driver w1_gpio_driver = {
	.driver = {
		.name	= "w1-gpio",
		.owner	= THIS_MODULE,
	},
	.remove	= __exit_p(w1_gpio_remove),
	.suspend = w1_gpio_suspend,
	.resume = w1_gpio_resume,
};

static int __init w1_gpio_init(void)
{
	return platform_driver_probe(&w1_gpio_driver, w1_gpio_probe);
}

static void __exit w1_gpio_exit(void)
{
	platform_driver_unregister(&w1_gpio_driver);
}

module_init(w1_gpio_init);
module_exit(w1_gpio_exit);

MODULE_DESCRIPTION("GPIO w1 bus master driver");
MODULE_AUTHOR("Ville Syrjala <syrjala@sci.fi>");
MODULE_LICENSE("GPL");
