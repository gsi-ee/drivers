#=========================< begin file & file header >=======================
#  References
#  
#    filename : Makefile
#    author   : JFG
#    company  : IOxOS
#    creation : may 15,2009 - nov 5 2015 
#    version  : 0.0.2
#
#----------------------------------------------------------------------------
#  Description
#
#    This Makefile allows to build the device driver module for the PEV1100
#    hotplug driver
#	 JAM: added appending of kernel version to module name
#
#----------------------------------------------------------------------------
#  Copyright Notice
#  
#    Copyright and all other rights in this document are reserved by 
#    IOxOS Technologies SA. This documents contains proprietary information    
#    and is supplied on express condition that it may not be disclosed, 
#    reproduced in whole or in part, or used for any other purpose other
#    than that for which it is supplies, without the written consent of  
#    IOxOS Technologies SA                                                        
#
#----------------------------------------------------------------------------
#  Change History
#  $Log : $
#  
#=============================< end file header >============================
obj-m := vetar.o wishbone.o

KERNELDIR = /rootfs_ppc/usr/src/linux
ARCH      = powerpc
CROSS_COMPILE = /tools_ppc/powerpc-linux-gnuspe-
EXTRA_CFLAGS += -DGIT_REVISION="598ee57 (EE-copy) 2017-02-09 11:10:54"

PWD := $(shell pwd)
default:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNELDIR) M=$(PWD) modules
	@mv wishbone.ko wishbone_$(shell uname -r).ko 
	@mv vetar.ko vetar_$(shell uname -r).ko 
clean:
	$(RM) *.o *.ko *.mod.c
