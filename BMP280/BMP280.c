/*
// Filename: bmp280_module.c
// Date: 2021-08-23
// Description: 
// <author>hunterteaegg
// <email>hunterteaegg@163.com
*/

#include <linux/module.h>
#include <linux/device.h>
#include <linux/device/class.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i2c.h>

#include <linux/cdev.h>
#include <linux/fs.h>

static struct i2c_adapter *i2c1_adapter;
static struct i2c_client *bmp280_client;
static struct class *sensor_class;
static struct device *bmp280_device;

static struct file_operations bmp280_cdev_fops;
static struct BMP280_driver bmp280_driver;

struct BMP280_driver
{
		struct i2c_driver bmp280_i2c_driver;
		struct cdev bmp280_cdev;
		dev_t cdev_number;
};	

static const struct i2c_device_id bmp280_id[] = {
		{ "bosch,bmp280", 0},
		{ },
};
MODULE_DEVICE_TABLE(i2c, bmp280_id);

static const struct i2c_board_info i2c1_devices[] = {
		{
				I2C_BOARD_INFO("bosch,bmp280", 0x76),
		},
};

static int bmp280_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
		int ret;

		ret = alloc_chrdev_region(&bmp280_driver.cdev_number, 0, 1, "bmp280_cdev");
		if(ret < 0)
		{
				printk(KERN_INFO "bosch,bmp280: major number allocation failed");
		}
		else
		{
				printk(KERN_INFO "bosch,bmp280: major number allocation successfully");
		}

		cdev_init(&bmp280_driver.bmp280_cdev, &bmp280_cdev_fops); 
		bmp280_driver.bmp280_cdev.owner = THIS_MODULE;
		bmp280_driver.bmp280_cdev.ops = &bmp280_cdev_fops;

		ret = cdev_add(&bmp280_driver.bmp280_cdev, bmp280_driver.cdev_number, 1);
		if(ret)
		{
				printk(KERN_NOTICE "bosch,bmp280: Error on cdev_add");
		}
		else
		{
		       printk(KERN_INFO "bosch,bmp280: cdev add successfully");
		}

		bmp280_device = device_create(sensor_class, NULL, bmp280_driver.cdev_number, NULL, "BMP280");

		printk(KERN_INFO "bosch,bmp280: probe");

		return 0;
}

static int bmp280_i2c_remove(struct i2c_client *client)
{
		device_destroy(sensor_class, bmp280_driver.cdev_number);
		cdev_del(&bmp280_driver.bmp280_cdev);
		unregister_chrdev_region(bmp280_driver.cdev_number, 1);
		printk(KERN_INFO "bosch,bmp280: remove");
		
		return 0;
}

static ssize_t bmp280_cdev_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
		char *string = "Your are reading BMP280.\n";

		return copy_to_user(buf, string, (count <= 24 ? count : 24));
}

static int bmp280_cdev_open(struct inode *inode, struct file *filp)
{
		;

		return 0;
}

static int bmp280_cdev_release(struct inode *inode, struct file *filp)
{
		;

		return 0;
}

static struct i2c_driver bmp280_i2c_driver = {
		.driver = {
				.owner = THIS_MODULE,
				.name = "bosch,bmp280",
		},
		.id_table = bmp280_id,
		.probe = bmp280_i2c_probe,
		.remove = bmp280_i2c_remove,
};

static struct file_operations bmp280_cdev_fops = {
		.owner = THIS_MODULE,
		.read = bmp280_cdev_read,
		.open = bmp280_cdev_open,
		.release = bmp280_cdev_release,
};

static struct BMP280_driver bmp280_driver = {
		.bmp280_i2c_driver = {
				.driver = {
						.owner = THIS_MODULE,
						.name = "bosch,bmp280",
				},
				.id_table = bmp280_id,
				.probe = bmp280_i2c_probe,
		        .remove = bmp280_i2c_remove,
		},	   
};

static int __init bmp280_init_module(void)
{
		i2c1_adapter = i2c_get_adapter(1);
		if(!i2c1_adapter)
		{
				printk(KERN_DEBUG "bosch,bmp280: get i2c1_adapter failed");
		}
		else
		{
				printk(KERN_DEBUG "bosch,bmp280: get i2c1_adapter successfully"); 
		}
		bmp280_client = i2c_new_client_device(i2c1_adapter, i2c1_devices);
		i2c_put_adapter(i2c1_adapter);

		sensor_class = class_create(THIS_MODULE, "sensors");
		
		printk(KERN_INFO "bosch,bmp280: init");
		
		return i2c_add_driver(&bmp280_i2c_driver);
}

static void __exit bmp280_exit_module(void)
{
		i2c_unregister_device(bmp280_client);
		i2c_del_driver(&bmp280_i2c_driver);

		class_destroy(sensor_class);

		printk(KERN_INFO "bosch,bmp280: exit");
}

module_init(bmp280_init_module);
module_exit(bmp280_exit_module);

MODULE_AUTHOR("Hunterteaegg <hunterteaegg@163.com>");
MODULE_DESCRIPTION("Driver for Bosch,BMP280");
MODULE_LICENSE("GPL");

