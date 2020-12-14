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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stdlib.h>
/***** Public Headers end *****/

/***** Private Headers start *****/
#include "bmp280.h"
/***** Private Headers end *****/

/***** Global Variable start *****/
int g_file; //file description
struct spi_ioc_transfer *g_msg = NULL;
uint8_t *g_rx_buff = NULL;
uint8_t *g_tx_buff = NULL;
typedef struct {
    uint32_t rawTemp;
    uint32_t rawPress;
} BMP280_RAWDATA_T;

BMP280_SETTINGS default_settings = {
    .powermode = POWER_MODE_NORMAL,
    .oversampling_temp = OVERSAMPLING_x1,
    .oversampling_press = OVERSAMPLING_x4,
    .standy_time = STANDBY_TIME_005,
    .filter_coefficient = FILTER_MODE_4,
    .spi3w = SPI3W_DISABLE,
};
/***** Global Variable end *****/

/***** Static funtion declearation start *****/
/*
 * @brief transmit data to specific address
 * @param addr The address that will be sent to
 * @param data The data that will be sent
 * @return {none}
 */
static void BMP280_TxData(uint8_t addr, uint8_t data);

#ifdef DEBUG
/*
 * @brief receive data from specific address
 * @param data The address that we want to receive from
 * @return the data on specific address
 */
static uint8_t BMP280_RxData(uint8_t addr);
#endif

/*
 * @brief get values from BMP280 compensation registers
 * @param {none}
 * @return {none}
 */
static void BMP280_getDig(void);

/*
 * @brief get raw data from BMP280
 * @param {none}
 * @return {none}
 */
static BMP280_RAWDATA_T BMP280_getRawData(void);

//compensation algorithm from datasheet
#ifdef FIX_POINT
static int32_t bmp280_compensate_T_int32(int32_t adc_T);
static uint32_t bmp280_compensate_P_int32(int32_t adc_P);
#endif
#ifdef FLOAT_POINT
typedef int32_t BMP280_S32_t;
static double bmp280_compensate_T_double(BMP280_S32_t adc_T);
static double bmp280_compensate_P_double(BMP280_S32_t adc_P);
#endif
/***** Static function declearation end *****/

int BMP280_init(BMP280_SETTINGS handle, int p_node, int c_node)
{

    //char pathName[20] by dynamic memory allocation
    char *pathName;
    pathName = (char *)malloc(20 * sizeof(char));
    int bit_per_word = 8;
    uint8_t mode;
    snprintf(pathName, 19, "/dev/spidev%d.%d", p_node, c_node);

    g_file = open(pathName ,O_RDWR);
    if(g_file < 0)
    {
        perror("open /dev/spidev*.* device failed. \n");
        exit(-1);
    }
#ifdef DEBUG
    printf("open /dev/spidev%d.%d successfully.\n", p_node, c_node);
#endif
    free(pathName);
    pathName = NULL;

    int res;
    res = ioctl(g_file, SPI_IOC_RD_MODE, &mode);
    if(res < 0)
    {
        printf("setting SPI rx Mode failed.\n");
        exit(-1);
    }
    res = ioctl(g_file, SPI_IOC_WR_MODE, &mode);
    if(res < 0)
    {
        printf("setting SPI tx Mode failed.\n");
        exit(-1);
    }
    res = ioctl(g_file, SPI_IOC_RD_BITS_PER_WORD, &bit_per_word);
    res = ioctl(g_file, SPI_IOC_WR_BITS_PER_WORD, &bit_per_word);

#ifdef DEBUG
    printf("setting SPI Mode successfully.\n");
    printf("SPI Mode are tx:%d, rx:%d.\n",mode,mode);
#endif
    
    //setting BMP280 work mode
    uint8_t config = ((uint8_t)(handle.standy_time)<<5) | ((uint8_t)(handle.filter_coefficient)<<2) | ((uint8_t)(handle.spi3w));
	uint8_t ctrl_meas = ((uint8_t)(handle.oversampling_temp)<<5) | ((uint8_t)(handle.oversampling_press)<<2) | ((uint8_t)(handle.powermode));

    BMP280_TxData(BMP280_CONFIG_REG, config);
    BMP280_TxData(BMP280_CTRL_MEAS_REG, ctrl_meas);

    sleep(1);

    return 0;
}

BMP280_DATA_T BMP280_getData(void)
{
    BMP280_DATA_T data;
    BMP280_getDig();
    BMP280_RAWDATA_T rawData = BMP280_getRawData();
#ifdef  FIX_POINT
    data.temp = bmp280_compensate_T_int32(rawData.rawTemp);
    data.press = bmp280_compensate_P_int32(rawData.rawPress);
#endif
#ifdef  FLOAT_POINT
    data.temp = bmp280_compensate_T_double(rawData.rawTemp);
    data.press = bmp280_compensate_P_double(rawData.rawPress);
#endif
    return data;
}
int BMP280_deinit(void)
{
    close(g_file);

#ifdef DEBUG
    printf("close spi device BMP280 successfully.\n");
#endif

    return 0;
}

static void BMP280_TxData(uint8_t addr, uint8_t data)
{
    g_tx_buff = (uint8_t *)malloc(2 * sizeof(uint8_t));
    if(NULL == g_tx_buff)
    {
        perror("memory allocation failed.\n");
        exit(-1);
    }
    g_tx_buff[0] = (addr & 0x7F);
    g_tx_buff[1] = data;

    g_msg = (struct spi_ioc_transfer *)malloc(1 * sizeof(struct spi_ioc_transfer));
    memset((void *)g_msg, 0, sizeof(g_msg));
    g_msg[0].tx_buf = (unsigned long long)g_tx_buff;
    g_msg[0].len = 2;
    g_msg[0].bits_per_word = 8;
    g_msg[0].speed_hz = 8 * 1000 * 1000;

    int res;
    res = ioctl(g_file, SPI_IOC_MESSAGE(1), g_msg);
    if(res < 0)
    {
        perror("SPI write failed.\n");
        exit(-1);
    }

#ifdef DEBUG
    printf("SPI write successfully.\n");
#endif

    free(g_tx_buff);
    g_tx_buff = NULL;
    free(g_msg);
    g_msg = NULL;
}

#ifdef DEBUG
static uint8_t BMP280_RxData(uint8_t addr)
{
    g_tx_buff = (uint8_t *)malloc(2 * sizeof(uint8_t));
    if(NULL == g_tx_buff)
    {
        perror("g_tx_buff memory allocation failed.\n");
        exit(-1);
    }

    g_rx_buff = (uint8_t *)malloc(2 * sizeof(uint8_t));
    if(NULL == g_rx_buff)
    {
        perror("g_rx_buff memory allocation failed.\n");
    }

    g_msg = (struct spi_ioc_transfer *)malloc(1 * sizeof(struct spi_ioc_transfer));
    if(NULL == g_msg)
    {
        perror("g_msg memory allocation failed.\n");
        exit(-1);
    }
    memset((void*)g_msg, 0, sizeof(g_msg));
    g_msg[0].tx_buf = (unsigned long long)g_tx_buff;
    g_msg[0].rx_buf = (unsigned long long)g_rx_buff;
    g_msg[0].len = 2;
    g_msg[0].bits_per_word = 8;
    g_msg[0].speed_hz = 8 * 1000 * 1000;

    //read chip ID, always return 0x58
    g_tx_buff[0] = (addr | 0x80);

    int res;
    res = ioctl(g_file, SPI_IOC_MESSAGE(1), g_msg);
    if(res < 0)
    {
        fprintf(stderr, "SPI read addr: 0x%02X failed.\n", addr);
        exit(-1);
    }

#ifdef DEBUG
    printf("SPI read successfully.\n");
    printf("The value of addr: 0x%02X is 0x%02X.\n", addr, g_rx_buff[1]);
#endif

    short returnValue = g_rx_buff[1];

    free(g_tx_buff);
    g_tx_buff = NULL;
    free(g_rx_buff);
    g_rx_buff = NULL;
    free(g_msg);
    g_msg = NULL;

    return returnValue;
}
#endif

static void BMP280_getDig(void)
{
    g_tx_buff = (uint8_t *)malloc((0xA1 - 0x88 + 1) * sizeof(uint8_t));
    if(NULL == g_tx_buff)
    {
        perror("g_tx_buff memory allocation failed.\n");
        exit(-1);
    }

    g_rx_buff = (uint8_t *)malloc((0xA1 - 0x88 + 1) * sizeof(uint8_t));
    if(NULL == g_rx_buff)
    {
        perror("g_rx_buff memory allocation failed.\n");
        exit(-1);
    }

    g_msg = (struct spi_ioc_transfer *)malloc(1 * sizeof(struct spi_ioc_transfer));
    if(NULL == g_msg)
    {
        perror("g_msg memory allocation failed.\n");
        exit(-1);
    }
    memset((void *)g_msg, 0, sizeof(g_msg));
    g_msg[0].tx_buf = (unsigned long long)g_tx_buff;
    g_msg[0].rx_buf = (unsigned long long)g_rx_buff;
    g_msg[0].len = (0xA1 - 0x88 + 1);
    g_msg[0].bits_per_word = 8;
    g_msg[0].speed_hz = 8 * 1000 * 1000;

    g_tx_buff[0] = 0x88;

    int res;
    res = ioctl(g_file, SPI_IOC_MESSAGE(1), g_msg);
    if(res < 0)
    {
        perror("Get dig register failed.\n");
        exit(-1);
    }
    
    dig_T1 = (g_rx_buff[1] | (g_rx_buff[2] << 8));
    dig_T2 = (g_rx_buff[3] | (g_rx_buff[4] << 8));
    dig_T3 = (g_rx_buff[5] | (g_rx_buff[6] << 8));
    dig_P1 = (g_rx_buff[7] | (g_rx_buff[8] << 8));
    dig_P2 = (g_rx_buff[9] | (g_rx_buff[10] << 8));
    dig_P3 = (g_rx_buff[11] | (g_rx_buff[12] << 8));
    dig_P4 = (g_rx_buff[13] | (g_rx_buff[14] << 8));
    dig_P5 = (g_rx_buff[15] | (g_rx_buff[16] << 8));
    dig_P6 = (g_rx_buff[17] | (g_rx_buff[18] << 8));
    dig_P7 = (g_rx_buff[19] | (g_rx_buff[20] << 8));
    dig_P8 = (g_rx_buff[21] | (g_rx_buff[22] << 8));
    dig_P9 = (g_rx_buff[23] | (g_rx_buff[24] << 8));

    free(g_tx_buff);
    g_tx_buff = NULL;
    free(g_rx_buff);
    g_rx_buff = NULL;
    free(g_msg);
    g_msg = NULL;
}

static BMP280_RAWDATA_T BMP280_getRawData(void)
{
    BMP280_RAWDATA_T rawData;
    g_tx_buff = (uint8_t *)malloc((0xFC - 0xF7 + 2) * sizeof(uint8_t));
    if(NULL == g_tx_buff)
    {
        perror("g_tx_buff memory allocation failed.\n");
        exit(-1);
    }

    g_rx_buff = (uint8_t *)malloc((0xFC - 0xF7 + 2) * sizeof(uint8_t));
    if(NULL == g_rx_buff)
    {
        perror("g_rx_buff memory allocation failed.\n");
        exit(-1);
    }

    g_msg = (struct spi_ioc_transfer *)malloc(1 * sizeof(struct spi_ioc_transfer));
    if(NULL == g_msg)
    {
        perror("g_msg memory allocation failed.\n");
        exit(-1);
    }

    memset((void *)g_msg, 0, sizeof(g_msg));
    g_msg[0].tx_buf = (unsigned long long)g_tx_buff;
    g_msg[0].rx_buf = (unsigned long long)g_rx_buff;
    g_msg[0].len = (0xFC - 0xF7 + 2);
    g_msg[0].bits_per_word = 8;
    g_msg[0].speed_hz = 8 * 1000 * 1000;

    g_tx_buff[0] = 0xF7;

    int res;
    res = ioctl(g_file, SPI_IOC_MESSAGE(1), g_msg);
    if(res < 0)
    {
        perror("Get raw data failed.\n");
        exit(-1);
    }

#ifdef DEBUG
    for(int i = 0;i < sizeof(g_rx_buff); i++)
    {
        printf("The value of rawData[%d] is 0x%02X.\n", i, g_rx_buff[i]);
    }
#endif

    rawData.rawPress = ((g_rx_buff[1] << 12) | (g_rx_buff[2] << 4) | (g_rx_buff[3] >> 4));
    rawData.rawTemp = ((g_rx_buff[4] << 12) | (g_rx_buff[5] << 4) | g_rx_buff[6] >> 4);

    free(g_tx_buff);
    g_tx_buff = NULL;
    free(g_rx_buff);
    g_rx_buff = NULL;
    free(g_msg);
    g_msg = NULL;

    return rawData;
}

#ifdef FIX_POINT
int32_t t_fine;
static int32_t bmp280_compensate_T_int32(int32_t adc_T)
{
	int32_t var1,var2,T;

	var1=((((adc_T>>3)-((int32_t)dig_T1<<1)))*((int32_t)dig_T2))>>11;
	var2=(((((adc_T>>4)-((int32_t)dig_T1))*((adc_T>>4)-((int32_t)dig_T1)))>>12)*((int32_t)dig_T3))>>14;
	t_fine=var1+var2;
	T=(t_fine*5+128)>>8;

	return T;
}

static uint32_t bmp280_compensate_P_int32(int32_t adc_P)
{
	int32_t var1,var2;
	uint32_t p;
	var1=(((int32_t)t_fine)>>1)-(int32_t)64000;
	var2=(((var1>>2)*(var1>>2))>>11)*((int32_t)dig_P6);
	var2=var2+((var1*((int32_t)dig_P5))<<1);
	var2=(var2>>2)+(((int32_t)dig_P4)<<16);
	var1=(((dig_P3*(((var1>>2)*(var1>>2))>>13))>>3)+((((int32_t)dig_P2)*var1)>>1))>>18;
	var1=((((32768+var1))*((int32_t)dig_P1))>>15);
	if(var1==0)
	{
		return 0;
	}
	p=(((uint32_t)(((int32_t)1048576)-adc_P)-(var2>>12)))*3125;
	if(p<0x80000000)
	{
		p=(p<<1)/((uint32_t)var1);
	}
	else
	{
		p=(p/(uint32_t)var1)*2;
	}
	var1=(((int32_t)dig_P9)*((int32_t)(((p>>3)*(p>>3))>>13)))>>12;
	var2=(((int32_t)(p>>2))*((int32_t)dig_P8))>>13;
	p=(uint32_t)((int32_t)p+((var1+var2+dig_P7)>>4));

	return p;
}
#endif

#ifdef FLOAT_POINT
BMP280_S32_t t_fine;
static double bmp280_compensate_T_double(BMP280_S32_t adc_T)
{
    double var1, var2, T;
    var1 = (((double)adc_T)/16384.0 - ((double)dig_T1)/1024.0) * ((double)dig_T2);
    var2 = ((((double)adc_T)/131072.0 - ((double)dig_T1)/8192.0) * (((double)adc_T)/131072.0 - ((double)dig_T1)/8192.0)) * ((double)dig_T3);
    t_fine = (BMP280_S32_t)(var1 + var2);
    T = (var1+var2) / 5120.0;

    return T;
}

static double bmp280_compensate_P_double(BMP280_S32_t adc_P)
{
    double var1, var2, p;
    var1 = ((double)t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
    var2 = var2 + var1 * ((double)dig_P5) * 2.0;
    var2 = (var2 / 4.0) + (((double)dig_P4) * 65536.0);
    var1 = (((double)dig_P3) * var1 * var1 / 524288.0 + ((double)dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double)dig_P1);
    if(var1 == 0.0)
    {
        return 0;
    }
    p = 1048576.0 - (double)adc_P;
    p = (p - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((double)dig_P9) * p * p / 2147483648.0;
    var2 = p * ((double)dig_P8) / 32768.0;
    p = p + (var1 + var2 + ((double)dig_P7)) / 16.0;

    return p;
}
#endif