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

/***** Public Headers begin*****/
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
#include "SHT31.h"
#include "../test/debug.h"
#include <string.h>

/***** Public Headers end *****/

/***** Type Definition start *****/
typedef uint16_t         u16;   //16-bitwidth datatype
/***** Type Definition end *****/


/***** Global Variable start *****/
static int g_file;                                 //device file descriptor
static int g_adapter_nr = 1;                       //device node
static SHT31_DATA_T g_data;                        //SHT31 final data
static struct i2c_msg g_msg;                       //single I2C transaction message
static struct i2c_rdwr_ioctl_data g_command_ioctl; //single I2C ioctl parameters to ioctl()
/***** Global Variable end *****/

/***** Public Interfaces start *****/
int SHT31_init(const int adapter_node)
{
    char filename[20];

    memset((void *)&g_msg, 0, sizeof(g_msg));
    snprintf(filename, 19, "/dev/i2c-%d", adapter_node);
    g_file = open(filename, O_RDWR);

    if (g_file < 0)
    {

#if DEBUG_SHT31
        fprintf(stderr, "open /dev/i2c-%d failed.\n", adapter_node);
#endif

        exit(-1);
    }

#if DEBUG_SHT31
    printf("open /dev/i2c-%d successfully.\n", adapter_node);
#endif

    //Initial g_command_ioctl
    g_command_ioctl.msgs = &g_msg;
    g_command_ioctl.nmsgs = (sizeof(g_msg) / sizeof(struct i2c_msg));

    return 0;
}

int SHT31_send_command(const u8 addr, const u8 com_MSB, const u8 com_LSB)
{
    int res;
    u8 command[2] =
    {
        com_MSB, com_LSB,
    };

    g_msg.addr = addr;
    g_msg.flags = 0; //write direction
    g_msg.buf = command;
    g_msg.len = sizeof(command);

    res = ioctl(g_file, I2C_SLAVE | I2C_RDWR, &g_command_ioctl);

    if (res < 0)
    {

#if DEBUG_SHT31
        fprintf(stderr, "send I2C command 0x%0X 0x%0X failed.\n", com_MSB, com_LSB);
#endif

        exit(-1);
    }

#if DEBUG_SHT31
    printf("send I2C command successfully.\n");
#endif

    return 0;
}

SHT31_DATA_T SHT31_read_data(const u8 addr)
{
    u8 buff[6] = {0};
    SHT31_DATA_T returnData = {0};

    g_msg.addr = addr;
    g_msg.flags = I2C_M_RD; //read direction
    g_msg.buf = buff,
    g_msg.len = sizeof(buff);

    int res;

    sleep(1);   //sleep for waiting for convertion
    res = ioctl(g_file, I2C_SLAVE | I2C_RDWR, &g_command_ioctl);
    if (res < 0)
    {
        perror("send I2C message failed.\n");
        exit(-1);
    }

#if DEBUG_SHT31
    printf("the first res is %d.\n", res);
#endif

    sleep(1);

#if DEBUG_SHT31
    for (int i = 0; i < sizeof(buff); i++)
    {
        printf("0x%0X ", buff[i]);
    }
    printf("\n");
#endif

    u16 temp_raw = ((u16)buff[0] << 8) | (u16)buff[1];
    double temp = (-45) + 175 * (temp_raw / (double)((0x01UL << 16) - 1));
    g_data.temp = temp;
    returnData.temp = temp;

    u16 humi_raw = ((u16)buff[3] << 8) | (u16)buff[4];
    double humi = 100 * (humi_raw / (double)((0x01UL << 16) - 1));
    g_data.humi = humi;
    returnData.humi = humi;

#if DEBUG_SHT31
    printf("the temp is %f.\n", temp);
    printf("the humi is %f.\n", humi);
    printf("crc temp is 0x%2X.\n", CRC8_compute(buff, 2, 0x31));
    printf("crc humi is 0x%2X.\n", CRC8_compute(buff + 3, 2, 0x31));
#endif

    return returnData;
}

int SHT31_deinit(void)
{
    return close(g_file);
}

u8 CRC8_compute(const u8 *pdat, short len, const u8 factor)
{
    u8 j;
    u8 crc = 0xFF;

    while (len--)
    {
        crc ^= (*pdat++);
        for (j = 8; j > 0; j--)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ factor;
            }
            else
            {
                crc = (crc << 1);
            }
        }
    }

    return crc;
}
/***** Public Interfaces end *****/