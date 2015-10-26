#=========================< begin file & file header >=======================
#  References
#  
#    filename : Makefile
#    author   : JFG
#    company  : IOxOS
#    creation : may 15,2009
#    version  : 0.0.1
#
#----------------------------------------------------------------------------
#  Description
#
#    This Makefile allows to build the device driver module for the PEV1100
#    hotplug driver
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

#trigmod-objs := vmetrigmod.o
KERNELDIR = /rootfs_ppc/usr/src/linux
ARCH      = powerpc
CROSS_COMPILE = /tools_ppc/powerpc-linux-gnuspe-
ADD_CFLAGS=-DPPC
PWD := $(shell pwd)
default:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	$(RM) *.o *.ko *.mod.c