/*
// Filename: bmp280_module.c
// Date: 2021-08-23
// Description: Bosch,BMP280 driver for personal use without commercial use
// <author>hunterteaegg
// <email>hunterteaegg@163.com
*/

#include <linux/module.h>
#include <linux/device.h>
#include <linux/device/class.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#define BMP280_ADDR             	0x76
#define BMP280_REG_TEMP_XLSB    	0xFC
#define BMP280_REG_TEMP_MSB			0xFA
#define BMP280_REG_PRESS_MSB    	0xF7
#define BMP280_REG_CONFIG       	0xF5
#define BMP280_REG_CTRLMEAS     	0xF4
#define BMP280_REG_STATUS       	0xF3
#define BMP280_REG_RESET        	0xE0
#define BMP280_REG_ID           	0xD0
#define BMP280_REG_COMPENSATE00 	0x88

static struct i2c_driver bmp280_driver;

static ssize_t bmp280_read_regs(struct device *dev, char *buff, int count, char reg_addr)
{
		struct i2c_client *client = to_i2c_client(dev); 
		char _reg_addr[1]; 
		
		_reg_addr[0] = reg_addr;
		i2c_master_send(client, _reg_addr, ARRAY_SIZE(_reg_addr));
		i2c_master_recv(client, buff, count);

		return (ssize_t)count;
}

static ssize_t bmp280_write_regs(struct device *dev, const char *buff, int count, char reg_addr)
{
		struct i2c_client *client = to_i2c_client(dev);
		char data[2];

		if(count != 1)
		{
				return -EINVAL;
		}
	    data[0] = reg_addr;
	    data[1] = buff[0];

		i2c_master_send(client, data, ARRAY_SIZE(data));

		return count;	
}

static ssize_t bmp280_attr_regs_temp_show(struct device *dev, struct device_attribute *attr, char *buff)
{
		return bmp280_read_regs(dev, buff, 3, BMP280_REG_TEMP_MSB);
}

static ssize_t bmp280_attr_regs_press_show(struct device *dev, struct device_attribute *attr, char *buff)
{
		return bmp280_read_regs(dev, buff, 3, BMP280_REG_PRESS_MSB);
}

static ssize_t bmp280_attr_reg_id_show(struct device *dev, struct device_attribute *attr, char *buff)
{
		return bmp280_read_regs(dev, buff, 1, BMP280_REG_ID);
}

static ssize_t bmp280_attr_regs_compensate_show(struct device *dev, struct device_attribute *attr, char *buff)
{
		return bmp280_read_regs(dev, buff, 24, BMP280_REG_COMPENSATE00);
}

static ssize_t bmp280_attr_reg_config_show(struct device *dev, struct device_attribute *attr, char *buff)
{
		return bmp280_read_regs(dev, buff, 1, BMP280_REG_CONFIG);
}

static ssize_t bmp280_attr_reg_config_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
		return bmp280_write_regs(dev, buff, count, BMP280_REG_CONFIG);
}

static ssize_t bmp280_attr_reg_ctrlmeas_show(struct device *dev, struct device_attribute *attr, char *buff)
{
		return bmp280_read_regs(dev, buff, 1, BMP280_REG_CTRLMEAS);
}

static ssize_t bmp280_attr_reg_ctrlmeas_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
		return bmp280_write_regs(dev, buff, count, BMP280_REG_CTRLMEAS);
}

static ssize_t bmp280_attr_reg_status_show(struct device *dev, struct device_attribute *attr, char *buff)
{
		return bmp280_read_regs(dev, buff, 1, BMP280_REG_STATUS);
}

static ssize_t bmp280_attr_reg_reset_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
		return bmp280_write_regs(dev, buff, count, BMP280_REG_RESET);
}

static DEVICE_ATTR(bmp280_reset, S_IWUSR, NULL, bmp280_attr_reg_reset_store);
static DEVICE_ATTR(bmp280_status, S_IRUGO, bmp280_attr_reg_status_show, NULL);
static DEVICE_ATTR(bmp280_ctrlmeas, (S_IRUGO | S_IWUSR), bmp280_attr_reg_ctrlmeas_show, bmp280_attr_reg_ctrlmeas_store);
static DEVICE_ATTR(bmp280_config, (S_IRUGO | S_IWUSR), bmp280_attr_reg_config_show, bmp280_attr_reg_config_store); 
static DEVICE_ATTR(bmp280_compensate, S_IRUGO, bmp280_attr_regs_compensate_show, NULL);
static DEVICE_ATTR(bmp280_id, S_IRUGO, bmp280_attr_reg_id_show, NULL);
static DEVICE_ATTR(bmp280_press, S_IRUGO, bmp280_attr_regs_press_show, NULL);
static DEVICE_ATTR(bmp280_temp, S_IRUGO, bmp280_attr_regs_temp_show, NULL);

static const struct attribute *bmp280_attrs[] = {
		&dev_attr_bmp280_reset.attr,
		&dev_attr_bmp280_status.attr,
		&dev_attr_bmp280_ctrlmeas.attr,
		&dev_attr_bmp280_config.attr,
		&dev_attr_bmp280_id.attr,
		&dev_attr_bmp280_press.attr,
		&dev_attr_bmp280_temp.attr,
		&dev_attr_bmp280_compensate.attr,
		NULL,
};

static struct of_device_id bmp280_of_devices[] = {
		{ .compatible = "My Bosch Sensor,bmp280", },
};

static int bmp280_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
		int ret;

		ret = sysfs_create_files(&client->dev.kobj, bmp280_attrs);
		if(!ret)
		{
				printk("sysfs create files successfully");
		}
		else
		{
				printk("sysfs create files failed");
		}
		printk(KERN_INFO "bosch,bmp280: probed");

		return 0;
}

static int bmp280_i2c_remove(struct i2c_client *client)
{
		sysfs_remove_files(&client->dev.kobj, bmp280_attrs);
		printk(KERN_INFO "bosch,bmp280: removed");
		
		return 0;
}

static struct i2c_driver bmp280_driver = {
		.driver = {
				.owner = THIS_MODULE,
				.name = "bosch,bmp280",
				.of_match_table = bmp280_of_devices,
		},
		.probe = bmp280_i2c_probe,
		.remove = bmp280_i2c_remove,
};

static int __init bmp280_init_module(void)
{
		printk(KERN_INFO "bosch,bmp280: init");
		
		return i2c_add_driver(&bmp280_driver);
}

static void __exit bmp280_exit_module(void)
{
		i2c_del_driver(&bmp280_driver);

		printk(KERN_INFO "bosch,bmp280: exit");
}

module_init(bmp280_init_module);
module_exit(bmp280_exit_module);

MODULE_AUTHOR("hunterteaegg <hunterteaegg@163.com>");
MODULE_DESCRIPTION("Driver for Bosch,BMP280");
MODULE_LICENSE("GPL");

