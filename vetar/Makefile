ifeq ($(TOPDIR),)
TOPDIR := /lib/modules/$(shell uname -r)/build
endif

ifneq ("$(SUBDIRS)", "")

#vetar-objs := vetar.o

obj-m := vetar.o

EXTRA_CFLAGS += -I$(SHL_ROOTDIR)/usr/include/ces
EXTRA_CFLAGS +=  -I$(SHL_ROOTDIR)/usr/include/ces/cesXpcLib
#EXTRA_CFLAGS += -DVETAR_NEW_XPCLIB
EXTRA_CFLAGS += -D__Linux__

else

UTSFILE := $(wildcard $(TOPDIR)/include/linux/version.h $(TOPDIR)/include/linux/utsrelease.h $(TOPDIR)/include/generated/utsrelease.h)
ifneq ($(UTSFILE),)
KVER := $(shell cat $(UTSFILE) | grep UTS_RELEASE | awk '{ print $$3 }' | sed 's/\"//g')
endif

all: vetar.ko

.PHONY: install clean vetar.ko

vetar.ko:
	make -C $(TOPDIR) SUBDIRS=$(PWD) modules


install:
	install -D -m 644 vetar.ko $(SHL_ROOTDIR)/lib/modules/$(KVER)/ces/vetar.ko

clean:
	rm -f *.o *.ko *.mod.c .*.cmd .*.cmd modules.order Module.symvers
	rm -rf .tmp_versions


endif
