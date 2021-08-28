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
#include <linux/errno.h>

#include <linux/cdev.h>
#include <linux/fs.h>

#define BMP280_ADDR             0x76
#define BMP280_REG_TEMP_XLSB    0xFC
#define BMP280_REG_TEMP_MSB		0xFA
#define BMP280_REG_PRESS_MSB    0xF7
#define BMP280_REG_CONFIG       0xF5
#define BMP280_REG_CTRLMEAS     0xF4
#define BMP280_REG_STATUS       0xF3
#define BMP280_REG_RESET        0xE0
#define BMP280_REG_ID           0xD0
#define BMP280_REG_Cab00        0x88

static struct i2c_adapter *i2c1_adapter;
static struct i2c_client *bmp280_client;
static struct class *sensor_class;
static struct device *bmp280_device;

static struct file_operations bmp280_cdev_fops;
static struct BMP280_driver bmp280_driver;

static ssize_t bmp280_attr_regs_temp_show(struct device *dev, struct device_attribute *attr, char *buff)
{
		char temp[3];
		char reg_addr[1] = { BMP280_REG_TEMP_MSB };
		i2c_master_send(bmp280_client, reg_addr, ARRAY_SIZE(reg_addr));
		i2c_master_recv(bmp280_client, temp, ARRAY_SIZE(temp));

		memcpy(buff, temp, ARRAY_SIZE(temp));

		return ARRAY_SIZE(temp);
}

static ssize_t bmp280_attr_regs_press_show(struct device *dev, struct device_attribute *attr, char *buff)
{
		char press[3];
		char reg_addr[1] = { BMP280_REG_PRESS_MSB };
		i2c_master_send(bmp280_client, reg_addr, ARRAY_SIZE(reg_addr));
		i2c_master_recv(bmp280_client, press, ARRAY_SIZE(press));

		memcpy(buff, press, ARRAY_SIZE(press));

		return ARRAY_SIZE(press);
}

static ssize_t bmp280_attr_reg_id_show(struct device *dev, struct device_attribute *attr, char *buff)
{
		char id[1];
		char reg_addr[1] = { BMP280_REG_ID };
		i2c_master_send(bmp280_client, reg_addr, ARRAY_SIZE(reg_addr));
		i2c_master_recv(bmp280_client, id, ARRAY_SIZE(id));

		memcpy(buff, id, ARRAY_SIZE(id));

		return ARRAY_SIZE(id);
}

static ssize_t bmp280_attr_regs_calliration_show(struct device *dev, struct device_attribute *attr, char *buff)
{
		char callibration[24];
		char reg_addr[1] = { BMP280_REG_Cab00 };

		i2c_master_send(bmp280_client, reg_addr, ARRAY_SIZE(reg_addr));
		i2c_master_recv(bmp280_client, callibration, ARRAY_SIZE(callibration));

		memcpy(buff, callibration, ARRAY_SIZE(callibration));

		return ARRAY_SIZE(callibration);
}

static ssize_t bmp280_attr_reg_config_show(struct device *dev, struct device_attribute *attr, char *buff)
{
		char config[1];
		char reg_addr[1] = { BMP280_REG_CONFIG };

		i2c_master_send(bmp280_client, reg_addr, ARRAY_SIZE(reg_addr));
		i2c_master_recv(bmp280_client, config, ARRAY_SIZE(config));

		memcpy(buff, config, ARRAY_SIZE(config));

		return ARRAY_SIZE(config);
}

static ssize_t bmp280_attr_reg_config_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
		char data[2];

		if(count != 1)
		{
			return 0;
		}
		data[0] = BMP280_REG_CONFIG;
		data[1] = buff[0];

		i2c_master_send(bmp280_client, data, ARRAY_SIZE(data));

		return count;
}

static ssize_t bmp280_attr_reg_ctrlmeas_show(struct device *dev, struct device_attribute *attr, char *buff)
{
		char ctrlmeas[1];
		char reg_addr[1] = { BMP280_REG_CTRLMEAS };

		i2c_master_send(bmp280_client, reg_addr, ARRAY_SIZE(reg_addr));
		i2c_master_recv(bmp280_client, ctrlmeas, ARRAY_SIZE(ctrlmeas));

		memcpy(buff, ctrlmeas, ARRAY_SIZE(ctrlmeas));

		return ARRAY_SIZE(ctrlmeas);
}

static ssize_t bmp280_attr_reg_ctrlmeas_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
		char data[2];

		if(count != 1)
		{
		    return 0;
		}
		data[0] = BMP280_REG_CTRLMEAS;
		data[1] = buff[0];
		i2c_master_send(bmp280_client, data, ARRAY_SIZE(data));

		return count;
}

static ssize_t bmp280_attr_reg_status_show(struct device *dev, struct device_attribute *attr, char *buff)
{
        char status[1];
		char reg_addr[1] = { BMP280_REG_STATUS };

		i2c_master_send(bmp280_client, reg_addr, ARRAY_SIZE(reg_addr));
		i2c_master_recv(bmp280_client, status, ARRAY_SIZE(status));

		memcpy(buff, status, ARRAY_SIZE(status));

		return ARRAY_SIZE(status);
}

static ssize_t bmp280_attr_reg_reset_store(struct device *dev, struct device_attribute *attr, const char *buff, size_t count)
{
		char data[2];
		if(!buff)
		{
				return 0;
		}
		data[0] = BMP280_REG_RESET;
		data[1] = buff[0];
		i2c_master_send(bmp280_client, data, ARRAY_SIZE(data));

		return count;
}

static DEVICE_ATTR(bmp280_reset, S_IWUSR, NULL, bmp280_attr_reg_reset_store);
static DEVICE_ATTR(bmp280_status, S_IRUGO, bmp280_attr_reg_status_show, NULL);
static DEVICE_ATTR(bmp280_ctrlmeas, (S_IRUGO | S_IWUSR), bmp280_attr_reg_ctrlmeas_show, bmp280_attr_reg_ctrlmeas_store);
static DEVICE_ATTR(bmp280_config, (S_IRUGO | S_IWUSR), bmp280_attr_reg_config_show, bmp280_attr_reg_config_store); 
static DEVICE_ATTR(bmp280_callibration, S_IRUGO, bmp280_attr_regs_calliration_show, NULL);
static DEVICE_ATTR(bmp280_id, S_IRUGO, bmp280_attr_reg_id_show, NULL);
static DEVICE_ATTR(bmp280_press, S_IRUGO, bmp280_attr_regs_press_show, NULL);
static DEVICE_ATTR(bmp280_temp, S_IRUGO, bmp280_attr_regs_temp_show, NULL);

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
				I2C_BOARD_INFO("bosch,bmp280", BMP280_ADDR),
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
		device_create_file(bmp280_device, &dev_attr_bmp280_temp);
		device_create_file(bmp280_device, &dev_attr_bmp280_press);
		device_create_file(bmp280_device, &dev_attr_bmp280_config);
		device_create_file(bmp280_device, &dev_attr_bmp280_ctrlmeas);
		device_create_file(bmp280_device, &dev_attr_bmp280_status);
		device_create_file(bmp280_device, &dev_attr_bmp280_reset);
		device_create_file(bmp280_device, &dev_attr_bmp280_id);
		device_create_file(bmp280_device, &dev_attr_bmp280_callibration);

		printk(KERN_INFO "bosch,bmp280: probe");

		return 0;
}

static int bmp280_i2c_remove(struct i2c_client *client)
{
		device_remove_file(bmp280_device, &dev_attr_bmp280_temp);
		device_remove_file(bmp280_device, &dev_attr_bmp280_id);
		device_remove_file(bmp280_device, &dev_attr_bmp280_press);
		device_remove_file(bmp280_device, &dev_attr_bmp280_config);
		device_remove_file(bmp280_device, &dev_attr_bmp280_ctrlmeas);
		device_remove_file(bmp280_device, &dev_attr_bmp280_callibration);
		device_remove_file(bmp280_device, &dev_attr_bmp280_status);
		device_remove_file(bmp280_device, &dev_attr_bmp280_reset);

        device_destroy(sensor_class, bmp280_driver.cdev_number);
		cdev_del(&bmp280_driver.bmp280_cdev);
		unregister_chrdev_region(bmp280_driver.cdev_number, 1);

		printk(KERN_INFO "bosch,bmp280: remove");
		
		return 0;
}

static ssize_t bmp280_cdev_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
		char reg_data[BMP280_REG_TEMP_XLSB - BMP280_REG_Cab00 + 1];

		if((*offset > BMP280_REG_TEMP_XLSB) || (*offset < BMP280_REG_Cab00))
		{
				return -EFAULT;
		}
		if(count + *offset > BMP280_REG_TEMP_XLSB + 1)
		{
				count = BMP280_REG_TEMP_XLSB - *offset + 1;
		}

		i2c_master_send(bmp280_client, (const char*)offset, 1);
		i2c_master_recv(bmp280_client, reg_data, count);

		return copy_to_user(buf, reg_data, count);
}

static int bmp280_cdev_open(struct inode *inode, struct file *filp)
{
		filp->f_pos = BMP280_REG_Cab00;
		
		return 0;
}

static int bmp280_cdev_release(struct inode *inode, struct file *filp)
{
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

