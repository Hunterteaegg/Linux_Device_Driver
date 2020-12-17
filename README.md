# Linux_Device_Driver

## LISENCE TERM
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

## DESCRIPTION
    This repository is used to store and backup some sensors or 
    devices drivers used in Linux of Raspberrypi.

    Here is my compilation envionment below: 
    - Kernel : aarch64 Linux 5.4.0-1023-raspi
    - OS : Ubuntu 20.04 focal
    - CPU : BCM2835 @ 4x 1.2GHz
    - Compiler : gcc v9.3.0

## DRIVER LIST
### SHT31 
    Humidity and Temperature Sensor
    Voltage : 2.15V - 5.5V
    Interface : I2C
### BMP280
    Digital Pressure and Temperature Sensor
    Voltage : 1.7V - 3.6V
    Interface : I2C and SPI
### GY-30 (BH1750FVI)
    Digital 16 bit Serial Output Type Ambient Light Sensor IC
    Voltage : 2.4V - 3.6V
    Interface : I2C