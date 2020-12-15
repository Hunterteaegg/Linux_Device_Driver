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
#include <stdint.h>
/***** Public Headers end *****/

/***** Private Macro Definition start *****/
//GY30 Slave Addess
#define GY30_ADDR_L             0x23
#define GY30_ADDR_H             0x5C
//GY30 Command
#define GY30_POWER_DOWN         0x00
#define GY30_POWER_ON           0x01
#define GY30_RESET              0x07
//GY30 Mode Candicate
#define GY30_CON_H_MODE         0x10
#define GY30_CON_H_MODE2        0x11
#define GY30_CON_L_MODE         0x13
#define GY30_ONETIME_H_MODE     0x20
#define GY30_ONETIME_H_MODE2    0x21
#define GY30_ONETIME_L_MODE     0x23

//GY30 Mode Selection
#define GY30_MODE               GY30_ONETIME_H_MODE
/***** Private Macro Definition end *****/

/***** Public Interfaces start *****/

/*
 * @brief init GY30 device
 * @param adapter_node I2C adapter device node located in /dev/i2c-*
 * @param addr The address of GY30
 * @param com The command sent to GY30
 * @return {none}
 */
void GY30_init(const int adapter_node, uint8_t addr, uint8_t com);

/*
 * @brief get data from GY30
 * @param addr The address of GY30
 * @return The data of GY30
 * @note different modes have different complements
 */
double GY30_getData(uint8_t addr);

/*
 * @brief deinit GY30
 * @param {none}
 * @return {none}
 */
void GY30_deinit(void);
