QMAKE = qmake-qt4

INC = $(MBSROOT)/inc

PEX_BINPATH=/mbs/driv/mbspex_$(GSI_OS_VERSION)_DEB/bin/
PEX_LIBPATH=/mbs/driv/mbspex_$(GSI_OS_VERSION)_DEB/lib/
PEX_INCPATH=/mbs/driv/mbspex_$(GSI_OS_VERSION)_DEB/include
PEX_INC = -I$(PEX_INCPATH)
PEX_LIBA = $(PEX_LIBPATH)/libmbspex.a

CFLAGS  = $(POSIX) -DLinux -Dunix -DV40 -DGSI_MBS -D$(GSI_CPU_ENDIAN) -D$(GSI_CPU_PLATFORM)




ifneq ($(shell which qmake 2>/dev/null),)
ifneq ($(shell qmake --version 2>&1 | grep "Qt version 4."),)
QTVERSION = 4
else
ifneq ($(shell which qmake-qt4 2>/dev/null),)
QTVERSION = 4
QMAKE = qmake-qt4
endif
endif
endif


.PHONY: all clean set gui cleanset cleangui install



ifneq ($(QTVERSION),4)

all:
	@echo "No qt4 found, Nyxor gui cannot be compiled"

clean:
	@echo "No qt4 found, Nyxor gui was not compiled"

else


all: gui


gui:
	@echo "Generating Makefile.qt"
	$(QMAKE) FebexGui.pro -o Makefile.qt "QMAKE_LFLAGS += -Wl,-rpath,$(PEX_LIBPATH)" "INCLUDEPATH += $(PEX_INCPATH)" "LIBS+= -L $(PEX_LIBPATH) -lmbspex"
	+$(MAKE) -f Makefile.qt



cleangui:
ifneq ($(wildcard Makefile.qt),)
	make -f Makefile.qt clean
endif
	@rm -f Makefile.qt
	@rm -rf .obj .moc
	@echo "Clean Nyxor Gui done"
endif

	
clean: cleangui

install: 
	@echo "Installing to $(PEX_BINPATH) ..."
	@ln -fs $(PWD)/FebexGui $(PEX_BINPATH)
