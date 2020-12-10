obj-m += taskinfo.o

KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.ko *.mod* *.o modules.* Module.symvers

install:
	insmod taskinfo.ko

remove:
	rmmod taskinfo
