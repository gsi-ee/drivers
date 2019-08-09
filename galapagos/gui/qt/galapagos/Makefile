
include ../gui.mk

GOSIPFRAME_INCPATH=../framework/


.PHONY: all nombspex clean set gui gui-nombspex cleanset cleangui install



ifeq ($(QTVERSION),3)

all:
	@echo "No qt4 or greater found, Galapagui cannot be compiled"

clean:
	@echo "No qt4 or greater found, Galapagui was not compiled"

else


all: gui

nombspex: gui-nogapg

gui:
	@echo "Generating Makefile.qt"
	$(QMAKE) GalapagosGui.pro -o Makefile.qt "DEFINES+=USE_GALAPAGOS_LIB" "QMAKE_LFLAGS += -Wl,-rpath,$(GAPG_LIBPATH)" "INCLUDEPATH += $(GAPG_INCPATH) $(GOSIPFRAME_INCPATH)" "LIBS+= -L $(GAPG_LIBPATH) -lgalapagos"
	+$(MAKE) -f Makefile.qt

gui-nogapg: 
	@echo "Generating Makefile.qt without galapagos lib"
	$(QMAKE) GalapagosGui.pro -o Makefile.qt 
	+$(MAKE) -f Makefile.qt


cleangui:
ifneq ($(wildcard Makefile.qt),)
	make -f Makefile.qt clean
endif
	@rm -f Makefile.qt
	@rm -rf .obj .moc
	@rm -rf GalapagosGui
	@echo "Clean Galapagos Gui done"
endif

	
clean: cleangui

install: 
	@echo "Installing to $(GAPG_BINPATH) ..."
	@ln -fs $(PWD)/GalapagosGui $(GAPG_BINPATH)
