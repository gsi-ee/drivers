#
# N.Kurz, EE, GSI, 20-Jul-2015: modified for kernel version 3.3.10
#

ifeq ($(TOPDIR),)
TOPDIR := /lib/modules/$(shell uname -r)/build
endif

ifneq ("$(SUBDIRS)", "")

trigmod-objs := vmetrigmod.o

obj-m := trigmod.o

EXTRA_CFLAGS += -I$(SHL_ROOTDIR)/usr/include/ces
EXTRA_CFLAGS +=  -I$(SHL_ROOTDIR)/usr/include/ces/cesXpcLib
EXTRA_CFLAGS += -DVMETRIGMOD_NEW_XPCLIB
EXTRA_CFLAGS += -D__Linux__

else

UTSFILE := $(wildcard $(TOPDIR)/include/linux/version.h $(TOPDIR)/include/linux/utsrelease.h $(TOPDIR)/include/generated/utsrelease.h)
ifneq ($(UTSFILE),)
KVER := $(shell cat $(UTSFILE) | grep UTS_RELEASE | awk '{ print $$3 }' | sed 's/\"//g')
endif

all: trigmod.ko

.PHONY: install clean trigmod.ko

trigmod.ko:
	make -C $(TOPDIR) SUBDIRS=$(PWD) modules


install:
	install -D -m 644 trigmod.ko $(SHL_ROOTDIR)/lib/modules/$(KVER)/ces/trigmod.ko

clean:
	rm -f *.o *.ko *.mod.c .*.cmd .*.cmd modules.order Module.symvers
	rm -rf .tmp_versions


endif
