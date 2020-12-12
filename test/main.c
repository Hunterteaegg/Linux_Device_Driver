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
#include "../SHT31/SHT31.h"
/***** Public Headers end *****/

int main()
{
    //SHT31 test
    SHT31_init(SHT31_NODE);
    SHT31_send_command(SHT31_ADDR, SHT31_COM_N_CS, SHT31_COM_N_CS_H);
    SHT31_DATA_T data = SHT31_read_data(SHT31_ADDR);
    SHT31_deinit();
    printf("The temperature is %.2f, the humidity is %.2f%%.\n", data.temp, data.humi);

    return 0;
}