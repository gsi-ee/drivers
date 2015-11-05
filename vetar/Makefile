ifeq ($(TOPDIR),)
TOPDIR := /lib/modules/$(shell uname -r)/build
endif

ifneq ("$(SUBDIRS)", "")

obj-m := vetar.o wishbone.o

#vetar-objs := vetar.o wishbone.o



EXTRA_CFLAGS += -I$(SHL_ROOTDIR)/usr/include/ces
EXTRA_CFLAGS +=  -I$(SHL_ROOTDIR)/usr/include/ces/cesXpcLib
#EXTRA_CFLAGS += -DVETAR_NEW_XPCLIB
EXTRA_CFLAGS += -D__Linux__

else

UTSFILE := $(wildcard $(TOPDIR)/include/linux/version.h $(TOPDIR)/include/linux/utsrelease.h $(TOPDIR)/include/generated/utsrelease.h)
ifneq ($(UTSFILE),)
KVER := $(shell cat $(UTSFILE) | grep UTS_RELEASE | awk '{ print $$3 }' | sed 's/\"//g')
endif

#all: vetar.ko wishbone.ko

.PHONY: clean all

all:
	make -C $(TOPDIR) SUBDIRS=$(PWD) modules
	@mv wishbone.ko wishbone_$(shell uname -r).ko 
	@mv vetar.ko vetar_$(shell uname -r).ko 

#install:
#	install -D -m 644 vetar_wb.ko $(SHL_ROOTDIR)/lib/modules/$(KVER)/ces/vetar_wb.ko

clean:
	rm -f *.o *.ko *.mod.c .*.cmd .*.cmd modules.order Module.symvers
	rm -rf .tmp_versions


endif
