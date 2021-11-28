#obj-m = gtp.o
obj-m = gtp_fw.o
KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) CONFIG_DEBUG_INFO=1 modules 
clean:
	rm -rf *.o *.cmd *.mod.c *.unsigned  .*.cmd .tmp_versions/
	rm -rf *.ko modules.order  Module.symvers
