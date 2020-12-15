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
#include <unistd.h>
/***** Public Headers end *****/

/***** Private Headers start *****/
#include "../SHT31/SHT31.h"
#include "../BMP280/bmp280.h"
#include "../GY30/GY30.h"
/***** Private Headers end *****/

int main()
{
    //SHT31 test
    SHT31_init(SHT31_NODE);
    SHT31_send_command(SHT31_ADDR, SHT31_COM_N_CS, SHT31_COM_N_CS_H);
    SHT31_DATA_T sht31_data = SHT31_read_data(SHT31_ADDR);
    SHT31_deinit();
    printf("The temperature is %.2f, the humidity is %.2f%%.\n", sht31_data.temp, sht31_data.humi);

    //BMP280 test
    BMP280_SETTINGS settings = {
    .powermode = POWER_MODE_NORMAL,
    .oversampling_temp = OVERSAMPLING_x1,
    .oversampling_press = OVERSAMPLING_x4,
    .standy_time = STANDBY_TIME_005,
    .filter_coefficient = FILTER_MODE_4,
    .spi3w = SPI3W_DISABLE,
    };
    BMP280_init(settings, 0, 0);
#ifdef DEBUG
    BMP280_RxData(0xD0);
#endif
    sleep(1);   //wait for convertion
    BMP280_DATA_T bmp280_data = BMP280_getData();
#ifdef  FIX_POINT
    printf("The temp is %d, the press is %d.\n", bmp280_data.temp, bmp280_data.press);
#endif
#ifdef  FLOAT_POINT
    printf("The temperature is %.2f, the pressure is %.2f hPa.\n", bmp280_data.temp, (bmp280_data.press / 100.0));
#endif
    BMP280_deinit();

    //GY30 test
    GY30_init(1, GY30_ADDR_L, GY30_MODE);
    printf("The luminance is %.2f lx.\n", GY30_getData(GY30_ADDR_L));
    GY30_deinit();

    return 0;
}