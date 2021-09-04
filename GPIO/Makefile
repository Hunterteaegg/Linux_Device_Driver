obj-m := led_gpio.o
CURRENT_PATH := $(shell pwd)
LINUX_KERNEL_PATH := /home/pi/linux

all:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
	dtc -@ -Hepapr -I dts -O dtb -o gpio.dtbo gpio.dts
clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
	rm *.dtbo

