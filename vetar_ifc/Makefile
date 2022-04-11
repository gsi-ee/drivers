obj-m := vetar.o wishbone.o


INCDIR = ../include
#KERNELDIR ?= /lib/modules/$(shell uname -r)/build
# different on ifc-1:
KERNELDIR ?= /usr/src/kernel

KBUILD_EXTRA_SYMBOLS = /mbs/driv/ifc/althea/ALTHEA7910/driver/Module.symvers
EXTRA_CFLAGS += -DGIT_REVISION="fallout v6.2.0"

PWD       := $(shell pwd)

ccflags-y +=-Wno-date-time 


all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)
	@mv wishbone.ko wishbone_$(shell uname -r).ko 
	@mv vetar.ko vetar_$(shell uname -r).ko 


clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.* modules.order

