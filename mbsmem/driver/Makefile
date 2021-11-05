obj-m	:= mbsmem.o

mbspex-objs := mbsmem.o   


INCDIR = ../include
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

#ccflags-y +=-Wno-error=date-time 
ccflags-y +=-Wno-date-time 


all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)
	@mv mbsmem.ko mbsmem_$(shell uname -r).ko 


clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.* modules.order

