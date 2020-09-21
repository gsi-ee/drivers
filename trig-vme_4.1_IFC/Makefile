obj-m := trigmod.o
trigmod-objs := vmetrigmod.o




INCDIR = ../include
#KERNELDIR ?= /lib/modules/$(shell uname -r)/build
# different on ifc-1:
KERNELDIR ?= /usr/src/kernel

KBUILD_EXTRA_SYMBOLS = /mbs/driv/ifc/althea/ALTHEA7910/driver/Module.symvers

PWD       := $(shell pwd)

#ccflags-y +=-Wno-error=date-time 
ccflags-y +=-Wno-date-time 


all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)
#	@mv vmetrigmod.ko mvetrigmod_$(shell uname -r).ko 
#	@echo "Installing userheaders ..."
#	@cp -vf pex_user.h $(INCDIR)/


clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.* modules.order
#	rm -f $(INCDIR)/pex_user.h

