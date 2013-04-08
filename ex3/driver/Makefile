EXTRA_CFLAGS += -Wall -std=gnu99 -I~/dev/avr32-buildroot/buildroot-avr32-v2.2.1/project_build_avr32/uclibc/linux-2.6.24/include/linux/

ifneq ($(KERNELRELEASE),)
	obj-m := stk1000io.o 
	stk1000io-objs += main.o leds.o switches.o
else
	BUILDROOT = ~/dev/avr32-buildroot/buildroot-avr32-v2.2.1
	KERNELDIR ?= $(BUILDROOT)/project_build_avr32/uclibc/linux-2.6.24
	PWD := $(shell pwd)

default: 
	make ARCH=avr32 CROSS_COMPILE=avr32-linux- -C $(KERNELDIR) M=$(PWD) modules

endif

clean: 
	rm -fr stk1000io.ko *.o Module.symvers .*.cmd *.mod.c .tmp_versions
