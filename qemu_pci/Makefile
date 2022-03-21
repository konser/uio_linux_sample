obj-m += edu_driver.o

KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
all:
	        $(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	        rm -rf *.ko *.mod* *.o modules.* Module.symvers

install:
	        insmod edu_driver.ko

remove:
	        rmmod edu_driver

