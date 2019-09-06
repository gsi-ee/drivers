

include ../gui.mk


.PHONY: all nogalapagos clean set gui gui-nogalapagos cleanset cleangui install



ifeq ($(QTVERSION),3)

all:
	@echo "No qt4 or greater found, Basic gui cannot be compiled"

clean:
	@echo "No qt4 or greater found, Basic gui was not compiled"

else


all: gui

nogalapagos: gui-nogalapagos

gui:
	@echo "Generating Makefile.qt"
#	$(QMAKE) BasicGui.pro -o Makefile.qt "DEFINES+=USE_GALAPAGOS_LIB" "QMAKE_LFLAGS += -Wl,-rpath,$(GAPG_LIBPATH)" "INCLUDEPATH += $(GAPG_INCPATH)" "LIBS+= -L $(GAPG_LIBPATH) -lgalapagos"
	$(QMAKE) BasicGui.pro -o Makefile.qt  "QMAKE_LFLAGS += -Wl,-rpath,$(GAPG_LIBPATH)" "INCLUDEPATH += $(GAPG_INCPATH)" "LIBS+= -L $(GAPG_LIBPATH) -lgalapagos"
	+$(MAKE) -f Makefile.qt

gui-nogalapagos: 
	@echo "Generating Makefile.qt without galapagos"
	$(QMAKE) BasicGui.pro -o Makefile.qt 
	+$(MAKE) -f Makefile.qt


cleangui:
ifneq ($(wildcard Makefile.qt),)
	make -f Makefile.qt clean
endif
	@rm -f Makefile.qt
	@rm -rf .obj .moc
	@echo "Clean Basic Gui done"
endif

	
clean: cleangui

install: 
	@echo "Installing to $(GAPG_BINPATH) ..."
	@ln -fs $(PWD)/BasicGui $(GAPG_BINPATH)
