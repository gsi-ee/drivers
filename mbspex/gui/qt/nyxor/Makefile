QMAKE = qmake-qt4

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


ifneq ($(QTVERSION),4)

all:
	@echo "No qt4 found, Nyxor gui cannot be compiled"

clean:
	@echo "No qt4 found, Nyxor gui was not compiled"

else


all:
	@echo "Generating Makefile.qt"
	$(QMAKE) NyxorGui.pro -o Makefile.qt "QMAKE_LFLAGS += -Wl,-rpath,/mbs/driv/mbspex_$(GSI_OS_VERSION)_DEB/lib/" "INCLUDEPATH += /mbs/driv/mbspex_$(GSI_OS_VERSION)_DEB/include" "LIBS+= -L /mbs/driv/mbspex_$(GSI_OS_VERSION)_DEB/lib -lmbspex"
	+$(MAKE) -f Makefile.qt

clean:
ifneq ($(wildcard Makefile.qt),)
	make -f Makefile.qt clean
endif
	@rm -f Makefile.qt
	@rm -rf .obj .moc
	@echo "Clean Nyxor Gui done"

endif

