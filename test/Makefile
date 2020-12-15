#/****** LICENSE TERM ******
#Copyright (C) <year>2020  <author>hunterteaegg <email>hunterteaegg@163.com
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.
#****** LICENSE TERM ******/

objects = main.o ../SHT31/SHT31.o ../GY30/GY30.o ../BMP280/BMP280.o


test : $(objects)
	cc -o test $(objects)
#main.o : debug.h main.c 
#	cc -c main.c
#BMP280.o : debug.h BMP280.h BMP280.c
#	cc -c BMP280.c
#GY30.o : debug.h GY30.h GY30.c
#	cc -c GY30.c
#SHT31.o : debug.h SHT31.h SHT31.c 
#	cc -c SHT31.c 

$(objects) : debug.h 
BMP280.o : BMP280.h 
GY30.o : GY30.h 
SHT31.o : SHT31.h 

.PHONY: clean
clean : 
	rm test $(objects)
