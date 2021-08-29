#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include "test.h"

static BMP280_S32_t t_fine;
static struct bmp280_handle_t bmp280_handle = {
		.powermode = POWER_MODE_NORMAL,
		.oversampling_temp = OVERSAMPLING_x1,
		.oversampling_press = OVERSAMPLING_x4,
		.standby_time = STANDBY_TIME_005,
		.filter_coefficient = FILTER_MODE_4,
		.spi3w = SPI3W_DISABLE,
};

static void get_calibration(void)
{
		int fd, num;
		char calibration[24];

		fd = open("/sys/devices/virtual/sensors/BMP280/bmp280_callibration", O_RDONLY);
		if(fd < 0)
		{
				perror("open calibration file failed.");
				exit(fd);
		}
		num = read(fd, calibration, 24);
		printf("calibration: read %d bytes\n", num);
		printf("calibration:");
		for(int i = 0; i < 24; i++)
		{
				printf("%d ", calibration[i]);
		}
		printf("\n");

		dig_T1 = (uint16_t)calibration[0] | ((uint16_t)calibration[1] << 8);
		dig_T2 = (uint16_t)calibration[2] | ((uint16_t)calibration[3] << 8);
		dig_T3 = (uint16_t)calibration[4] | ((uint16_t)calibration[5] << 8);
		dig_P1 = (uint16_t)calibration[6] | ((uint16_t)calibration[7] << 8);
		dig_P2 = (uint16_t)calibration[8] | ((uint16_t)calibration[9] << 8);
		dig_P3 = (uint16_t)calibration[10] | ((uint16_t)calibration[11] << 8);
		dig_P4 = (uint16_t)calibration[12] | ((uint16_t)calibration[13] << 8);
		dig_P5 = (uint16_t)calibration[14] | ((uint16_t)calibration[15] << 8);
		dig_P6 = (uint16_t)calibration[16] | ((uint16_t)calibration[17] << 8);
		dig_P7 = (uint16_t)calibration[18] | ((uint16_t)calibration[19] << 8);
		dig_P8 = (uint16_t)calibration[20] | ((uint16_t)calibration[21] << 8);
		dig_P9 = (uint16_t)calibration[22] | ((uint16_t)calibration[23] << 8);
}

static double bmp280_compensate_T_double(BMP280_S32_t adc_T)
{
		double var1, var2, T;

		var1 = (((double)adc_T) / 16384.0 - ((double)dig_T1) / 1024.0) * ((double)dig_T2);
		var2 = ((((double)adc_T) / 131072.0 - ((double)dig_T1) / 8192.0) * \
								(((double)adc_T) / 131072.0 - ((double)dig_T1) / 8192.0)) * \
								 ((double)dig_T3);
		t_fine = (BMP280_S32_t)(var1 + var2);
		T = (var1 + var2) / 5120.0;

		return T;
}

static double bmp280_compensate_P_double(BMP280_S32_t adc_P)
{
		double var1, var2, p;

		var1 = ((double)t_fine / 2.0) - 64000.0;
		var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
		var2 = var2 + var1 * ((double)dig_P5) * 2.0;
		var2 = (var2/4.0) + (((double)dig_P4) * 65536.0);
		var1 = (((double)dig_P3) * var1 * var1 / 524288.0 + \
						((double)dig_P2) * var1) / 524288.0;
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

int bmp280_init(struct bmp280_handle_t *handle)
{
		char ctrl_meas[1], config[1];
		int fd;

		config[0] = (handle->standby_time << 5) | \
				 (handle->filter_coefficient << 2) | \
				 (handle->spi3w);
		ctrl_meas[0] = (handle->oversampling_temp << 5) | \
				 (handle->oversampling_press << 2) | \
				 (handle->powermode);
		get_calibration();
		fd = open("/sys/devices/virtual/sensors/BMP280/bmp280_config", O_WRONLY);
		if(fd < 0)
		{
				perror("open bmp280_config failed");
		}
		write(fd, config, 1);
		close(fd);

		fd = open("/sys/devices/virtual/sensors/BMP280/bmp280_ctrlmeas", O_WRONLY);
		if(fd < 0)
		{
				perror("open bmp280_ctrlmeas failed");
		}
		write(fd, ctrl_meas, 1);
		close(fd);
}

struct bmp280_data_t bmp280_getdata(void)
{
		struct bmp280_data_t data;
		char temp[3];
		char press[3];
		uint32_t temp_raw, press_raw;
		int fd;

		fd = open("/sys/devices/virtual/sensors/BMP280/bmp280_temp", O_RDONLY);
		if(fd < 0)
		{
				perror("open bmp280_temp failed");
		}
		read(fd, temp, 3);
		close(fd);

		fd = open("/sys/devices/virtual/sensors/BMP280/bmp280_press", O_RDONLY);
		if(fd < 0)
		{
				perror("open bmp280_press failed");
		}
		read(fd, press, 3);
		close(fd);

		temp_raw = (temp[0] << 12) | (temp[1] << 4) | (temp[2] >> 4);
		press_raw = (press[0] << 12) | (press[1] << 4) | (press[2] >> 4);

		data.temp = bmp280_compensate_T_double(temp_raw);
		data.press = bmp280_compensate_P_double(press_raw);

		return data;
}

int main()
{
		struct bmp280_data_t data;

		bmp280_init(&bmp280_handle);
		data = bmp280_getdata();

		printf("temp: %f, press:%f\n", data.temp, data.press);
		
		return 0;
}

