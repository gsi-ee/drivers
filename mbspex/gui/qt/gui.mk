QMAKE = qmake-qt4

ifndef PEXORSYS

INC = $(MBSROOT)/inc

PEX_BASE=/mbs/driv/mbspex_$(GSI_OS_VERSION)_DEB
#PEX_BASE=/daq/usr/adamczew/workspace/drivers/mbspex
PEX_BINPATH=$(PEX_BASE)/bin/
PEX_LIBPATH=$(PEX_BASE)/lib/
PEX_INCPATH=$(PEX_BASE)/include
PEX_INC = -I$(PEX_INCPATH)
PEX_LIBA = $(PEX_LIBPATH)/libmbspex.a

else

#INC = $(PEXORSYS)/user/include

PEX_BASE=/$(PEXORSYS)/user/
PEX_BINPATH=$(PEX_BASE)/bin/
PEX_LIBPATH=$(PEX_BASE)/lib/
PEX_INCPATH=$(PEX_BASE)/include
PEX_INC = -I$(PEX_INCPATH)
PEX_LIBA = $(PEX_LIBPATH)/libmbspex.a


endif



CFLAGS  = $(POSIX) -DLinux -Dunix -DV40 -DGSI_MBS -D$(GSI_CPU_ENDIAN) -D$(GSI_CPU_PLATFORM)
