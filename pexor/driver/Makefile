obj-m	:= pexor.o

pexor-objs := pexor_base.o pexor_gos.o pexor_trb.o 



INCDIR = $(PEXORSYS)/user/include
KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)
 


all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)
	@echo "Installing userheaders ..."
	@cp -vf pexor_user.h $(INCDIR)/


clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.* modules.order
	rm -f $(INCDIR)/pexor_user.h

