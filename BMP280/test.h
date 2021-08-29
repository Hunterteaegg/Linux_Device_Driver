#ifndef __TEST_H__
#define __TEST_H__

#include <stdint.h>

typedef int32_t BMP280_S32_t;

static unsigned short dig_T1;
static signed short dig_T2;
static signed short dig_T3;
static unsigned short dig_P1;
static signed short dig_P2;
static signed short dig_P3;
static signed short dig_P4;
static signed short dig_P5;
static signed short dig_P6;
static signed short dig_P7;
static signed short dig_P8;
static signed short dig_P9;

enum bmp280_power_mode
{
		POWER_MODE_SLEEP = 0,
		POWER_MODE_FORCE = 1,
		POWER_MODE_NORMAL = 3,
};

enum bmp280_oversampling
{
		OVERSAMPLING_SKIP = 0,
		OVERSAMPLING_x1 = 1,
		OVERSAMPLING_x2 = 2,
		OVERSAMPLING_x4 = 3,
		OVERSAMPLING_x8 = 4,
		OVERSAMPLING_x16 = 5,
};

enum bmp280_standby_time
{
		STANDBY_TIME_005 = 0,
		STANDBY_TIME_625 = 1,
		STANDBY_TIME_125 = 2,
		STANDBY_TIME_250 = 3,
		STANDBY_TIME_500 = 4,
		STANDBY_TIME_1000 = 5,
		STANDBY_TIME_2000 = 6,
		STANDBY_TIME_4000 = 7,
};

enum bmp280_filter_coefficient
{
		FILTER_OFF = 0x0,
		FILTER_MODE_1,
		FILTER_MODE_2,
		FILTER_MODE_3,
		FILTER_MODE_4,
};

enum bmp280_spi3w
{
		SPI3W_ENABLE = 1,
		SPI3W_DISABLE = 0,
};

struct bmp280_data_t
{
		double temp;
		double press;
};

struct bmp280_handle_t
{
		enum bmp280_power_mode powermode;
		enum bmp280_oversampling oversampling_temp;
		enum bmp280_oversampling oversampling_press;
		enum bmp280_standby_time standby_time;
		enum bmp280_filter_coefficient filter_coefficient;
		enum bmp280_spi3w spi3w;

		struct bmp280_data_t data;
};

int bmp280_init(struct bmp280_handle_t *handle);
struct bmp280_data_t bmp280_getdata(void);

#endif
