
QTVERSION = 3

ifneq ($(shell which qmake 2>/dev/null),)
ifneq ($(shell qmake -qt5 --version 2>&1 | grep "Qt version 5."),)
QTVERSION = 5
QMAKE = qmake -qt=5
else
ifneq ($(shell qmake --version 2>&1 | grep "Qt version 4."),)
QTVERSION = 4
QMAKE = qmake
else
ifneq ($(shell which qmake-qt4 2>/dev/null),)
QTVERSION = 4
QMAKE = qmake-qt4
endif
endif
endif
endif



ifndef GALSYS

INC = $(MBSROOT)/inc

#GAPG_BASE=/mbs/driv/mbspex_$(GSI_OS_VERSION)_DEB
GAPG_BASE=/daq/usr/adamczew/workspace/drivers/galapagos
GAPG_BINPATH=$(GAPG_BASE)/bin/
GAPG_LIBPATH=$(GAPG_BASE)/lib/
GAPG_INCPATH=$(GAPG_BASE)/include
GAPG_INC = -I$(GAPG_INCPATH)
GAPG_LIBA = $(GAPG_LIBPATH)/libmbspex.a

else

#INC = $(GALSYS)/user/include

GAPG_BASE=/$(GALSYS)/user/
GAPG_BINPATH=$(GAPG_BASE)/bin/
GAPG_LIBPATH=$(GAPG_BASE)/lib/
GAPG_INCPATH=$(GAPG_BASE)/include
GAPG_INC = -I$(GAPG_INCPATH)
GAPG_LIBA = $(GAPG_LIBPATH)/libmbspex.a


endif



CFLAGS  = $(POSIX) -DLinux -Dunix -DV40 -DGSI_MBS -D$(GSI_CPU_ENDIAN) -D$(GSI_CPU_PLATFORM)
