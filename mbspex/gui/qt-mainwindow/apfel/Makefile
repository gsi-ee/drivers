 

include ../gui.mk


.PHONY: all nombspex clean set gui gui-nombspex cleanset cleangui install



ifeq ($(QTVERSION),3)

all:
	@echo "No qt4 or greater found, Apfel gui cannot be compiled"

clean:
	@echo "No qt4 or greater found, Apfel gui was not compiled"

else


all: gui

nombspex: gui-nombspex

gui:
	@echo "Generating Makefile.qt"
	$(QMAKE) ApfelGui.pro -o Makefile.qt "DEFINES+=USE_MBSPEX_LIB" "QMAKE_LFLAGS += -Wl,-rpath,$(PEX_LIBPATH) -Wl,--no-as-needed" "INCLUDEPATH += $(PEX_INCPATH)" "LIBS+=  -L $(PEX_LIBPATH) -lmbspex"
	+$(MAKE) -f Makefile.qt

gui-nombspex: 
	@echo "Generating Makefile.qt without mbspex"
	$(QMAKE) ApfelGui.pro -o Makefile.qt 
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
	@ln -fs $(PWD)/ApfelGui $(PEX_BINPATH)
