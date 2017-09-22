obj-m	:= mbspex.o

mbspex-objs := pex_base.o pex_gosip.o  



INCDIR = ../include
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

#ccflags-y +=-Wno-error=date-time 
ccflags-y +=-Wno-date-time 


all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)
	@mv mbspex.ko mbspex_$(shell uname -r).ko 
	@echo "Installing userheaders ..."
	@cp -vf pex_user.h $(INCDIR)/


clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.* modules.order
	rm -f $(INCDIR)/pex_user.h

