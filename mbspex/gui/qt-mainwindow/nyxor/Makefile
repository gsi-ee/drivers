
include ../gui.mk

GOSIPFRAME_INCPATH=../framework/


.PHONY: all clean set gui cleanset cleangui install installgui



ifeq ($(QTVERSION),3)

all:
	@echo "No qt4 or greater found, Nyxor gui cannot be compiled"

clean:
	@echo "No qt4 or greater found, Nyxor gui was not compiled"

else


all: set gui


gui:
	@echo "Generating Makefile.qt"
	$(QMAKE) NyxorGui.pro -o Makefile.qt "DEFINES+=USE_MBSPEX_LIB" "QMAKE_LFLAGS += -Wl,-rpath,$(PEX_LIBPATH) -Wl,--no-as-needed" "INCLUDEPATH += $(PEX_INCPATH) $(GOSIPFRAME_INCPATH)" "LIBS+= -L $(PEX_LIBPATH) -lmbspex"
	+$(MAKE) -f Makefile.qt

set: m_set_nxy.c
	gcc -O2 $(CFLAGS) -o m_set_nxy -I$(INC) $(PEX_INC) m_set_nxy.c $(LIBS) $(PEX_LIBA)  


cleangui:
ifneq ($(wildcard Makefile.qt),)
	make -f Makefile.qt clean
endif
	@rm -f Makefile.qt
	@rm -rf .obj .moc
	@echo "Clean Nyxor Gui done"
endif

cleanset:
	@rm -f *.o m_set_nxy
	@echo "Clean m_set_nxy done"
	
clean: cleanset cleangui	


installgui: 
	@echo "Installing nyxor gui to $(PEX_BINPATH) ..."
	@ln -fs $(PWD)/NyxorGui $(PEX_BINPATH)

install: installgui
	@echo "Installing tools to $(PEX_BINPATH) ..."
	@cp -f m_set_nxy $(PEX_BINPATH)
