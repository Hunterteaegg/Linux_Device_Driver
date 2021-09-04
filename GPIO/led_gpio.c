#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/device/driver.h>
#include <linux/mod_devicetable.h>
#include <linux/fs.h>
#include <linux/gpio/consumer.h>
#include <linux/string.h>

static struct gpio_desc *gpio_4;

static ssize_t led_show(struct device *dev, struct device_attribute *attr, char *buff)
{
		return sysfs_emit(buff, "led_show");
}

static ssize_t led_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
		char data[1];

		memcpy(data, buff, 1);

		if(strncmp(data, "0", 1) == 0)
		{
				gpiod_direction_output(gpio_4, 0);
		}
		else if(strncmp(data, "1", 1) == 0)
		{
				gpiod_direction_output(gpio_4, 1);
		}
		else
		{
				return 0;
		}

		return 1;
}

static DEVICE_ATTR(led_ctrl, S_IWUSR | S_IRUSR, led_show, led_store);

static struct of_device_id led_of_match[] = {
		{ .compatible = "My Device,LED", },
};

static int led_probe(struct platform_device *device)
{
		int ret;

		gpio_4 = gpiod_get_optional(&device->dev, "led", GPIOD_OUT_HIGH);
		if(!gpio_4)
		{
				printk(KERN_INFO "led-gpio: get gpio failed");
		}
		else
		{
				printk(KERN_INFO "led-gpio: get gpio successfully");
		}
		gpiod_direction_output(gpio_4, 1);

		ret = sysfs_create_file(&device->dev.kobj, &dev_attr_led_ctrl.attr);
		if(!ret)
		{
				printk(KERN_DEBUG "led-gpio: sysfs create file successfully");
		}
		else
		{
				printk(KERN_DEBUG "led-gpio: sysfs create file failed");
		}
		printk(KERN_DEBUG "led-gpio: probe");

		return 0;
}

static int led_remove(struct platform_device *device)
{
		sysfs_remove_file(&device->dev.kobj, &dev_attr_led_ctrl.attr);
		if(gpio_4)
		{
				gpiod_put(gpio_4);
		}
		
		printk(KERN_DEBUG "led-gpio: remove");

		return 0;
}

static struct platform_driver led_driver = {
		.probe = led_probe,
		.remove = led_remove,
		.driver = {
				.name = "LED",
				.owner = THIS_MODULE,
				.of_match_table = led_of_match,
		},
};

static int __init gpio_init_module(void)
{
		int ret;

		ret = platform_driver_register(&led_driver);
		if(!ret)
		{
				printk(KERN_INFO "led-gpio: platform driver register successfully");
		}
		else
		{
				printk(KERN_INFO "led-gpio: platform driver register failed");
		}
		printk(KERN_INFO "led-gpio: init");

		return 0;
}

static void __exit gpio_exit_module(void)
{
		platform_driver_unregister(&led_driver);

		printk(KERN_INFO "led-gpio: exit");
}

module_init(gpio_init_module);
module_exit(gpio_exit_module);

MODULE_AUTHOR("hunterteaegg <hunterteaegg@163.com>");
MODULE_DESCRIPTION("Driver for LED");
MODULE_LICENSE("GPL");

