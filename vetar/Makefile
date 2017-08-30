ifeq ($(TOPDIR),)
TOPDIR := /lib/modules/$(shell uname -r)/build
endif

KV := $(shell uname -r)

ifneq ("$(SUBDIRS)", "")

obj-m := vetar.o wishbone.o

# since we do not have git on build machine, use tag from latest source of wishbone.c module:
EXTRA_CFLAGS +=  -DGIT_REVISION="598ee57 (EE-copy) 2017-02-09 11:10:54"


EXTRA_CFLAGS += -I$(SHL_ROOTDIR)/usr/include/ces
EXTRA_CFLAGS +=  -I$(SHL_ROOTDIR)/usr/include/ces/cesXpcLib
#EXTRA_CFLAGS += -DVETAR_NEW_XPCLIB

ifeq ($(KV), 2.6.33.7-shl-3.3-up)
EXTRA_CFLAGS += -DVETAR_NEW_XPCLIB
EXTRA_CFLAGS += -DVETAR_VTRANS
endif

EXTRA_CFLAGS += -D__Linux__


else

UTSFILE := $(wildcard $(TOPDIR)/include/linux/version.h $(TOPDIR)/include/linux/utsrelease.h $(TOPDIR)/include/generated/utsrelease.h)
ifneq ($(UTSFILE),)
KVER := $(shell cat $(UTSFILE) | grep UTS_RELEASE | awk '{ print $$3 }' | sed 's/\"//g')
endif


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
