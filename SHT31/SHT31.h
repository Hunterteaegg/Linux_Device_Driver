#ifndef __SHT31_H__
#define __SHT31_H__

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

/***** Private Headers start *****/
#include "../test/debug.h"
/***** Private Headers end *****/

/***** Type Definition start *****/
typedef unsigned char    u8;
typedef struct {
    double temp;    //temperature
    double humi;    //humidity
} SHT31_DATA_T;
/***** Type Definition end *****/

/***** Private Marco Definition start *****/
#define SHT31_ADDR                  0x44    //SHT31 Slave Address
#define SHT31_NODE                  1       //SHT31 Slave Node

//SHT31 Single_Shot Mode Command
#define SHT31_COM_CS                0x2C    //Clock stretching (MSB)
#define SHT31_COM_CS_H              0x06    //Clock stretching with high repeatability
#define SHT31_COM_CS_M              0x0D    //Clock stretching with medium repeatability
#define SHT31_COM_CS_L              0x10    //Clock stretching with low repeatability
#define SHT31_COM_N_CS              0x24    //non-Clock stretching (MSB)
#define SHT31_COM_N_CS_H            0x00    //non-Clock stretching with high repeatability
#define SHT31_COM_N_CS_M            0x0B    //non-Clock stretching with medium repeatability
#define SHT31_COM_N_CS_L            0x16    //non-Clock stretching with low repeatability

//SHT31 Periodic Data Acquisition Mode Command
#define SHT31_COM_0_5               0x20    //0.5 mps (MSB)
#define SHT31_COM_0_5_H             0x32    //0.5 mps with high repeatability
#define SHT31_COM_0_5_M             0x24    //0.5 mps with medium repeatability
#define SHT31_COM_0_5_L             0x2F    //0.5 mps with low repeatability
#define SHT31_COM_1                 0x21    //1 mps (MSB)
#define SHT31_COM_1_H               0x30
#define SHT31_COM_1_M               0x26
#define SHT31_COM_1_L               0x2D
#define SHT31_COM_2                 0x22    //2 mps (MSB)
#define SHT31_COM_2_H               0x36    
#define SHT31_COM_2_M               0x20
#define SHT31_COM_2_L               0x2B
#define SHT31_COM_4                 0x23    //4 mps (MSB)
#define SHT31_COM_4_H               0x34
#define SHT31_COM_4_M               0x22
#define SHT31_COM_4_L               0x29
#define SHT31_COM_10                0x27    //10 mps (MSB)
#define SHT31_COM_10_H              0x37
#define SHT31_COM_10_M              0x21
#define SHT31_COM_10_L              0x2A
#define SHT31_COM_FETCH_DATA_M      0xE0    //fetch data command (MSB)
#define SHT31_COM_FETCH_DATA_L      0x00    //fetch data command (LSB)
#define SHT31_COM_ART_M             0x2B    //ART command (MSB)
#define SHT31_COM_ART_L             0x32    //ART command (LSB)

//SHT31 Control Command
#define SHT31_COM_BREAK_M           0x30    //break Command (MSB)
#define SHT31_COM_BREAK_L           0x93    //break Command (LSB)
#define SHT31_COM_RESET_M           0x30    //reset Command (MSB)
#define SHT31_COM_RESET_L           0xA2    //reset Command (LSB)
/***** Private Macro Definition end *****/

/****** Public Interfaces ******/
/* @brief Init SHT31 device.
 * @param adapter_node I2C_adapter node, can be seen in /dev/i2c-*
 * @return 0 means successed but -1 means failed. 
 */
int SHT31_init(const int adapter_node);

/* @brief Send SHT31 command.
 * @param addr Device address depends on physical pin connection. 
 * @param com_MSB The MSB of command ready to send.
 * @param com_LSB The LSB of command ready to send. 
 * @return 0 means successed but -1 means failed. 
 * @note The command is 16 bit-width.
 */
int SHT31_send_command(const u8 addr, const u8 com_MSB, const u8 com_LSB);

/* @brief Read SHT31 data
 * @param addr Device address depends on physical pin connection. 
 * @return A structure consist of temperature and humidity.
 */
SHT31_DATA_T SHT31_read_data(const u8 addr);

/* @brief Deinit SHT31 devices
 * @param {none}
 * @return 0 means successed but -1 means failed.
 */
int SHT31_deinit(void);

/* @brief CRC-8 calculation
 * @param pdat The pointer to data array
 * @param len The length of data array 
 * @param factor The polynomial of CRC-8
 * @return The result of CRC-8 of sample
 */
u8 CRC8_compute(const u8 *pdat, const short len, const u8 factor);

#endif