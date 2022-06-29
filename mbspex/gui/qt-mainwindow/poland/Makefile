
include ../gui.mk


GOSIPFRAME_INCPATH=../framework/



.PHONY: all nombspex clean set gui gui-nombspex cleanset cleangui install



ifeq ($(QTVERSION),3)

all:
	@echo "No qt4 or greater found, Poland gui cannot be compiled"

clean:
	@echo "No qt4 or greater found, Poland gui was not compiled"

else


all: gui

nombspex: gui-nombspex

ifndef PEXORSYS
gui:
	@echo "Generating Makefile.qt"
	$(QMAKE) PolandGui.pro -o Makefile.qt "DEFINES+=USE_MBSPEX_LIB" "QMAKE_LFLAGS += -Wl,-rpath,$(PEX_LIBPATH) -Wl,--no-as-needed" "INCLUDEPATH += $(PEX_INCPATH) $(GOSIPFRAME_INCPATH)" "LIBS+= -L $(PEX_LIBPATH) -lmbspex"
	+$(MAKE) -f Makefile.qt
else
gui:
	@echo "Generating Makefile.qt"
	$(QMAKE) PolandGui.pro -o Makefile.qt "DEFINES+=USE_MBSPEX_LIB USE_PEXOR_LIB" "QMAKE_LFLAGS += -Wl,-rpath,$(PEX_LIBPATH) -Wl,--no-as-needed" "INCLUDEPATH += $(PEX_INCPATH) $(GOSIPFRAME_INCPATH)" "LIBS+= -L $(PEX_LIBPATH) -lmbspex -lpexor"
	+$(MAKE) -f Makefile.qt
endif


gui-nombspex: 
	@echo "Generating Makefile.qt without mbspex"
	$(QMAKE) PolandGui.pro -o Makefile.qt 
	+$(MAKE) -f Makefile.qt


cleangui:
ifneq ($(wildcard Makefile.qt),)
	make -f Makefile.qt clean
endif
	@rm -f Makefile.qt
	@rm -rf .obj .moc
	@echo "Clean Poland Gui done"
endif

	
clean: cleangui

install: 
	@echo "Installing to $(PEX_BINPATH) ..."
	@ln -fs $(PWD)/PolandGui $(PEX_BINPATH)
