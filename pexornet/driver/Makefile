obj-m	:= pexornet.o

pexornet-objs := pexornet_base.o pexornet_gosip.o 



INCDIR = $(PEXORSYS)/include
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)



all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)
	@echo "Installing userheaders ..."
	@mkdir -p $(INCDIR)
	@cp -vfu pexornet_user.h $(INCDIR)/


clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.* modules.order
	rm -f $(INCDIR)/pexornet_user.h

