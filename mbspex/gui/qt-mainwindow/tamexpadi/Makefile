
include ../gui.mk

GOSIPFRAME_INCPATH=../framework/


.PHONY: all nombspex clean set gui gui-nombspex cleanset cleangui install



ifeq ($(QTVERSION),3)

all:
	@echo "No qt4 or greater found, TamexPadi gui cannot be compiled"

clean:
	@echo "No qt4 or greater found, TamexPadi gui was not compiled"

else


all: gui

nombspex: gui-nombspex

gui:
	@echo "Generating Makefile.qt"
	$(QMAKE) TamexPadiGui.pro -o Makefile.qt "DEFINES+=USE_MBSPEX_LIB" "QMAKE_LFLAGS += -Wl,-rpath,$(PEX_LIBPATH)" "INCLUDEPATH += $(PEX_INCPATH) $(GOSIPFRAME_INCPATH)" "LIBS+= -L $(PEX_LIBPATH) -lmbspex"
	+$(MAKE) -f Makefile.qt

gui-nombspex: 
	@echo "Generating Makefile.qt without mbspex"
	$(QMAKE) TamexPadiGui.pro -o Makefile.qt 
	+$(MAKE) -f Makefile.qt


cleangui:
ifneq ($(wildcard Makefile.qt),)
	make -f Makefile.qt clean
endif
	@rm -f Makefile.qt
	@rm -rf .obj .moc
	@echo "Clean TamexPadi Gui done"
endif

	
clean: cleangui

install: 
	@echo "Installing to $(PEX_BINPATH) ..."
	@ln -fs $(PWD)/TamexPadiGui $(PEX_BINPATH)
