
QTVERSION = 3

ifneq ($(shell which qmake 2>/dev/null),)
ifneq ($(shell qmake-qt5 --version 2>&1 | grep "Qt version 5."),)
QTVERSION = 5
QMAKE = qmake-qt5
else
ifneq ($(shell qmake -qt5 --version 2>&1 | grep "Qt version 5."),)
QTVERSION = 5
QMAKE = qmake -qt5
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
endif



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
