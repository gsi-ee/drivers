# N.Kurz, EE, GSI, 4-Sep-2013: modifed for different kernel versions

obj-m	:= pexor.o

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

all:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)
	chmod 755 pexor.ko  
	mv pexor.ko pexor_$(shell uname -r).ko 

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

