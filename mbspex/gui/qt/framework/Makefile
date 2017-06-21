

include ../gui.mk


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


.PHONY: all nombspex clean set gui gui-nombspex cleanset cleangui install



ifneq ($(QTVERSION),4)

all:
	@echo "No qt4 found, Gosip gui cannot be compiled"

clean:
	@echo "No qt4 found, Gosip gui was not compiled"

else


all: gui

nombspex: gui-nombspex

gui:
	@echo "Generating Makefile.qt"
	$(QMAKE) GosipGui.pro -o Makefile.qt "DEFINES+=USE_MBSPEX_LIB" "QMAKE_LFLAGS += -Wl,-rpath,$(PEX_LIBPATH)" "INCLUDEPATH += $(PEX_INCPATH)" "LIBS+= -L $(PEX_LIBPATH) -lmbspex"
	+$(MAKE) -f Makefile.qt

gui-nombspex: 
	@echo "Generating Makefile.qt without mbspex"
	$(QMAKE) GosipGui.pro -o Makefile.qt 
	+$(MAKE) -f Makefile.qt


cleangui:
ifneq ($(wildcard Makefile.qt),)
	make -f Makefile.qt clean
endif
	@rm -f Makefile.qt
	@rm -rf .obj .moc
	@echo "Clean Gosip Gui done"
endif

	
clean: cleangui

install: 
	@echo "Installing to $(PEX_BINPATH) ..."
	@ln -fs $(PWD)/GosipGui $(PEX_BINPATH)
