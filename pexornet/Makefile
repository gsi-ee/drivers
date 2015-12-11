# this is the top level Makefile of the pexornet package
# JAM 4-Nov-2015
#

	PEXORSYS ?= $(PWD)
	export PEXORSYS	


VERSION = 0.01

PEXORMODULE = pexornet.ko
PEXORHEADER = pexornet_user.h
PEXORLIBINSTALL = libpexornet.so


PEXORDRIVER = $(PEXORSYS)/driver
PEXORLIB  = $(PEXORSYS)/library
PEXORTEST  = $(PEXORSYS)/applications
PEXORCMD  = $(PEXORSYS)/gosipcmd

PEXORPACKAGE = PexorNetDriver

PACKDIR = pexornet_$(VERSION)
PACKAGEFILES = $(PACKDIR)/driver/ $(PACKDIR)/library $(PACKDIR)/applications $(PACKDIR)/gosipcmd $(PACKDIR)/common.mk $(PACKDIR)/Makefile $(PACKDIR)/README.txt



INSTALLDIR ?= /lib/modules/$(shell uname -r)/extra

LIBINSTALLDIR ?= /usr/lib64


.PHONY: all clean driver lib examples install uninstall driver-install driver-uninstall lib-install lib-uninstall pack

all: driver lib examples 

install: driver-install lib-install

uninstall: driver-uninstall lib-uninstall


clean:
	cd $(PEXORCMD); make clean
	cd $(PEXORTEST); make clean
	cd $(PEXORLIB); make clean
	cd $(PEXORDRIVER); make clean
	

driver:
	cd $(PEXORDRIVER); make all

lib: driver
	cd $(PEXORLIB); make all
	
examples: lib
	cd $(PEXORTEST); make all
	cd $(PEXORCMD); make all

driver-install:
	@mkdir -p $(INSTALLDIR)
	@echo "INSTALL $(INSTALLDIR)/$(PEXORMODULE)"
	@install -m 755 $(PEXORDRIVER)/$(PEXORMODULE) $(INSTALLDIR)
	@echo "INSTALL /usr/include/$(PEXORHEADER)"
	@mkdir -p /usr/include/
	@install -m 644 $(PEXORDRIVER)/$(PEXORHEADER) /usr/include/


driver-uninstall:
	@echo "UNINSTALL $(INSTALLDIR)/$(PEXORMODULE)"
	@rm -f $(INSTALLDIR)/$(PEXORMODULE)
	@echo "UNINSTALL /usr/include/$(PEXORHEADER)"
	@rm -rf /usr/include/$(PEXORHEADER)


lib-install:
	@mkdir -p $(LIBINSTALLDIR)
	@echo "INSTALL $(LIBINSTALLDIR)/$(PEXORLIBINSTALL)"
	@install -m 755 $(PEXORSYS)/user/lib/$(PEXORLIBINSTALL) $(LIBINSTALLDIR)
	@echo "INSTALL /usr/include/pexor"
	@mkdir -p /usr/include/pexor
	@install -m 644 $(PEXORSYS)/user/include/pexor/* /usr/include/pexor


lib-uninstall:
	@echo "UNINSTALL $(LIBINSTALLDIR)/$(PEXORLIBINSTALL)"
	@rm -f $(LIBINSTALLDIR)/$(PEXORLIBINSTALL)
	@echo "UNINSTALL /usr/include/pexor"
	@rm -rf /usr/include/pexor



pack: clean 
	@mkdir -p $(PACKDIR)
	@cp -rfd  $(PEXORSYS)/driver $(PACKDIR)
	@cp -rfd  $(PEXORSYS)/library $(PACKDIR)
	@cp -rfd  $(PEXORSYS)/applications $(PACKDIR)
	@cp -rfd  $(PEXORSYS)/gosipcmd $(PACKDIR)
	@cp -rfd  $(PEXORSYS)/Makefile $(PACKDIR)
	@cp -rfd  $(PEXORSYS)/README.txt $(PACKDIR)
	@cp -rfd  $(PEXORSYS)/README_DABC.txt $(PACKDIR)
	@rm -f $(PEXORPACKAGE).*
	@tar chvf $(PEXORPACKAGE).tar  $(PACKAGEFILES) --exclude=".svn" 
	@gzip $(PEXORPACKAGE).tar
	@rm -rf $(PACKDIR)

