 /****** LICENSE TERM ******
 Copyright (C) <year>2020  <author>hunterteaegg <email>hunterteaegg@163.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
****** LICENSE TERM ******/

/***** Public Headers start *****/
#include <stdio.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "GY30.h"
#include "../test/debug.h"
#include <string.h>
/***** Public Headers end *****/

#include "GY30.h"

/***** Global Variable start *****/
static int g_file;
static struct i2c_msg *g_msg;
static struct i2c_rdwr_ioctl_data *g_ioctl_data;
/***** Global Variable end *****/

void GY30_init(const int adapter_node, uint8_t addr, uint8_t com)
{
    char pathName[20];
    uint8_t command = com;
    snprintf(pathName, 19, "/dev/i2c-%d", adapter_node);
    g_file = open(pathName, O_RDWR);
    if(g_file < 0)
    {
        fprintf(stderr, "open /dev/i2c-%d failed.\n", adapter_node);
        exit(-1);
    }

#if DEBUG_GY30
    printf("open /dev/i2c-%d successfully.\n", adapter_node);
#endif

    g_msg = (struct i2c_msg *)malloc(1 * sizeof(struct i2c_msg));
    if(NULL == g_msg)
    {
        fprintf(stderr, "memory allocation failed.\n");
        exit(-1);
    }
    g_msg->addr = addr;
    g_msg->flags = 0;
    g_msg->buf = &command;
    g_msg->len = sizeof(command);

    g_ioctl_data = (struct i2c_rdwr_ioctl_data *)malloc(1 * sizeof(struct i2c_rdwr_ioctl_data));
    if(NULL == g_ioctl_data)
    {
        fprintf(stderr, "memory allocation failed.\n");
        exit(-1);
    }
    g_ioctl_data->msgs = g_msg;
    g_ioctl_data->nmsgs = (sizeof(*g_msg) / sizeof(struct i2c_msg));

    int res;
    res = ioctl(g_file, I2C_SLAVE | I2C_RDWR, g_ioctl_data);
    if(res < 0)
    {
        fprintf(stderr, "send I2C message failed.\n");
        exit(-1);
    }

#if DEBUG_GY30
    printf("send I2C init message successfully.\n");
#endif

#if ((GY30_MODE & 0x03) == 0x03)
    usleep(50 * 1000);
#else
    usleep(180 * 1000);
#endif

    free(g_msg);
    g_msg = NULL;
    free(g_ioctl_data);
    g_ioctl_data = NULL;
}

double GY30_getData(uint8_t addr)
{
    int res;
    g_msg = (struct i2c_msg *)malloc(1 * sizeof(struct i2c_msg));
    if(NULL == g_msg)
    {
        fprintf(stderr, "memory allocation failed.\n");
        exit(-1);
    }
    g_ioctl_data = (struct i2c_rdwr_ioctl_data *)malloc(1 * sizeof(struct i2c_rdwr_ioctl_data));
    if(NULL == g_ioctl_data)
    {
        fprintf(stderr, "memory allocation failed.\n");
        exit(-1);
    }

#if     ((GY30_MODE & 0x10) == 0x10)    //continuous measurement
    ;
#elif   ((GY30_MODE & 0x20) == 0x20)    //one-time measurement
    //Send Power-On Command
    uint8_t command = GY30_POWER_ON;
    
    g_msg->addr = addr;
    g_msg->flags = 0;
    g_msg->buf = &command;
    g_msg->len = sizeof(command);
    g_ioctl_data->msgs = g_msg;
    g_ioctl_data->nmsgs = (sizeof(*g_msg) / sizeof(struct i2c_msg));

    res = ioctl(g_file, I2C_SLAVE | I2C_RDWR, g_ioctl_data);
    if(res < 0)
    {
        fprintf(stderr, "send I2C power-on message failed.\n");
        exit(-1);
    }
#if DEBUG_GY30
    printf("send I2C power-on message successfully.\n");
#endif
    
#if ((GY30_MODE & 0x03) == 0x03)
    usleep(50 * 1000);
#else
    usleep(180 * 1000);
#endif

#else
    fprintf(stderr, "GY30 Mode Selection failed.\n");
    exit(-1);
#endif
    //Read Data
    uint16_t data;
    uint8_t buff[2] = {0};

    g_msg->addr = addr;
    g_msg->buf = buff;
    g_msg->flags = I2C_M_RD;
    g_msg->len = sizeof(buff);

    g_ioctl_data->msgs = g_msg;
    g_ioctl_data->nmsgs = (sizeof(*g_msg) / sizeof(struct i2c_msg));

    res = ioctl(g_file, I2C_SLAVE | I2C_RDWR, g_ioctl_data);
    if(res < 0)
    {
        fprintf(stderr, "send I2C read message failed.\n");
        exit(-1);
    }
#if DEBUG_GY30
    printf("send I2C read message succcessfully.\n");
#endif

    data = ((buff[0] << 8) | buff[1]);
#if DEBUG_GY30
    printf("The light is %f.\n", (data / (double)1.2));
#endif

    free(g_msg);
    g_msg = NULL;
    free(g_ioctl_data);
    g_ioctl_data = NULL;

    return (double)data / (double)1.2;
}

void GY30_deinit(void)
{
    int res = close(g_file);
    if(res < 0)
    {
        fprintf(stderr, "close device file failed.\n");
        exit(-1);
    }
}
