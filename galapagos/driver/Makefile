obj-m	:= galapagos.o

galapagos-objs := gapg_base.o   



INCDIR = ../include
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

#ccflags-y +=-Wno-error=date-time 
ccflags-y +=-Wno-date-time 


all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)
	@mv galapagos.ko galapagos_$(shell uname -r).ko 
	@echo "Installing userheaders ..."
	@cp -vf gapg_user.h $(INCDIR)/


clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.* modules.order
	rm -f $(INCDIR)/gapg_user.h

