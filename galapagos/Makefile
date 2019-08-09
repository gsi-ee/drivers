# this is the top level Makefile of the galapagos package
# JAM 7-Aug-2017
#

	GALSYS ?= $(PWD)
	export GALSYS	


VERSION = 1.01

GALMODULE = galapagos.ko
GALHEADER = galapagos_user.h
GALLIBINSTALL = libgalapagos.so


GALDRIVER = $(GALSYS)/driver
GALLIB  = $(GALSYS)/library
GALTEST  = $(GALSYS)/tools

GALPACKAGE = GalapagosDriver

PACKDIR = galapagos_$(VERSION)
PACKAGEFILES = $(PACKDIR)/driver/ $(PACKDIR)/library $(PACKDIR)/tests $(PACKDIR)/common.mk $(PACKDIR)/Makefile $(PACKDIR)/README.txt



INSTALLDIR ?= /lib/modules/$(shell uname -r)/extra

LIBINSTALLDIR ?= /usr/lib64


.PHONY: all clean driver lib examples install uninstall driver-install driver-uninstall lib-install lib-uninstall pack

all: driver lib examples 

install: driver-install lib-install

uninstall: driver-uninstall lib-uninstall


clean:

	cd $(GALTEST); make clean
	cd $(GALLIB); make clean
	cd $(GALDRIVER); make clean
	

driver:
	cd $(GALDRIVER); make all

lib: driver
	cd $(GALLIB); make all
	
examples: lib
	cd $(GALTEST); make all

driver-install:
	@mkdir -p $(INSTALLDIR)
	@echo "INSTALL $(INSTALLDIR)/$(GALMODULE)"
	@install -m 755 $(GALDRIVER)/$(GALMODULE) $(INSTALLDIR)
	@echo "INSTALL /usr/include/$(GALHEADER)"
	@mkdir -p /usr/include/
	@install -m 644 $(GALDRIVER)/$(GALHEADER) /usr/include/


driver-uninstall:
	@echo "UNINSTALL $(INSTALLDIR)/$(GALMODULE)"
	@rm -f $(INSTALLDIR)/$(GALMODULE)
	@echo "UNINSTALL /usr/include/$(GALHEADER)"
	@rm -rf /usr/include/$(GALHEADER)


lib-install:
	@mkdir -p $(LIBINSTALLDIR)
	@echo "INSTALL $(LIBINSTALLDIR)/$(GALLIBINSTALL)"
	@install -m 755 $(GALSYS)/user/lib/$(GALLIBINSTALL) $(LIBINSTALLDIR)
	@echo "INSTALL /usr/include/galapagos"
	@mkdir -p /usr/include/galapagos
	@install -m 644 $(GALSYS)/user/include/galapagos/* /usr/include/galapagos


lib-uninstall:
	@echo "UNINSTALL $(LIBINSTALLDIR)/$(GALLIBINSTALL)"
	@rm -f $(LIBINSTALLDIR)/$(GALLIBINSTALL)
	@echo "UNINSTALL /usr/include/galapagos"
	@rm -rf /usr/include/galapagos



pack: clean 
	@mkdir -p $(PACKDIR)
	@cp -rfd  $(GALSYS)/driver $(PACKDIR)
	@cp -rfd  $(GALSYS)/user $(PACKDIR)
	@cp -rfd  $(GALSYS)/Makefile $(PACKDIR)
	@cp -rfd  $(GALSYS)/README.txt $(PACKDIR)
	@cp -rfd  $(GALSYS)/README_DABC.txt $(PACKDIR)
	@rm -f $(GALPACKAGE).*
	@tar chvf $(GALPACKAGE).tar  $(PACKAGEFILES) --exclude=".svn" 
	@gzip $(GALPACKAGE).tar
	@rm -rf $(PACKDIR)

